#include <memory/memory.h>
#include <utils/console.h>
#include <utils/file_system.h>
#include <compilers/texture_compiler.h>
#include <compilers/wave_compiler.h>
#include <thread>

enum class Type : uint32_t
{
  kTypeUnknown,
  kInputKeyboard,
  kInputMouseButton,
  kInputMouseMove,
  kRendererResize,
  kRendererDraw,
};

struct IMessage
{
  IMessage() : type(Type::kTypeUnknown) {}
  IMessage(Type type) : type(type) {}

  Type type;
};

class IListener
{
public:
  virtual void Dispatch(IMessage* message) = 0;
};

class ChannelMessageManager
{
public:
  ChannelMessageManager()
  {
    RegisterChannel("Default");
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///// CHANNEL HANDLING //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint32_t RegisterChannel(const lambda::String& name)
  {
    // Empty name.
    if (name.empty())
      return 0u;

    // Already exists.
    auto it = channel_names_.find(name);
    if (it != channel_names_.end())
      return it->second;

    // If there is an empty channel. Use that channel.
    if (!empty_channels_.empty())
    {
      channel_names_.insert(eastl::make_pair(name, empty_channels_.back()));
      empty_channels_.pop_back();
      return channel_names_[name];
    }

    // Add the channel to the channel list.
    channel_names_.insert(eastl::make_pair(name, (uint32_t)channels_.size()));
    channels_.push_back();
    return channel_names_[name];
  }
  void UnregisterChannel(const lambda::String& name)
  {
    // Doesn't exist.
    auto it = channel_names_.find(name);
    if (it == channel_names_.end())
      return;
    
    // Remove all callbacks.
    channels_[it->second].clear();

    // Remove this channel from the registered channels.
    channel_names_.erase(it);

    // Add this channel to the unused list.
    empty_channels_.push_back(it->second);
  }
  uint32_t GetChannel(const lambda::String& name)
  {
    auto it = channel_names_.find(name);
    if (it != channel_names_.end())
      return it->second;
    else
      return 0u;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///// CHANNEL MESSAGING /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void RegisterListener(uint32_t channel, IListener* listener, void(*callback)(IListener*, IMessage*))
  {
    channels_[channel][listener].push_back(callback);
  }
  void RegisterListener(const lambda::String& channel, IListener* listener, void(*callback)(IListener*, IMessage*))
  {
    RegisterListener(GetChannel(channel), listener, callback);
  }
  void UnregisterListener(uint32_t channel, IListener* listener)
  {
    channels_[channel].erase(listener);
  }
  void UnregisterListener(const lambda::String& channel, IListener* listener)
  {
    UnregisterListener(GetChannel(channel), listener);
  }
  void Dispatch(uint32_t channel, IMessage* message)
  {
    for (const auto& listener : channels_[channel])
      for (const auto& callback : listener.second)
        callback(listener.first, message);
  }
  void Dispatch(const lambda::String& channel, IMessage* message)
  {
    Dispatch(GetChannel(channel), message);
  }

private:
  lambda::Map<lambda::String, uint32_t> channel_names_;
  lambda::Vector<lambda::Map<IListener*, lambda::Vector<void(*)(IListener*, IMessage*)>>> channels_;
  lambda::Vector<uint32_t> empty_channels_;
};


class GlobalMessageManager
{
public:
  GlobalMessageManager()
  {
  }

  void RegisterListener(IListener* listener)
  {
    listeners_.push_back(listener);
  }
  void UnregisterListener(IListener* listener)
  {
    auto it = eastl::find(listeners_.begin(), listeners_.end(), listener);
    if (it != listeners_.end())
      listeners_.erase(it);
  }
  void Dispatch(IMessage* message)
  {
    for (IListener* listener : listeners_)
      listener->Dispatch(message);
  }

private:
  lambda::Vector<IListener*> listeners_;
};


struct CMessage : public IMessage
{
  CMessage() : IMessage(Type::kRendererDraw) {}
  lambda::String payload;
};

class CListener : public IListener
{
public:
  CListener() : IListener() {}
  void HandleCMessage(IMessage* message)
  {
    const CMessage& msg = *(CMessage*)message;
    std::cout << "\t" << msg.payload.c_str() << std::endl;
  }
  void HandleCMessagePRIO(IMessage* message)
  {
    const CMessage& msg = *(CMessage*)message;
    std::cout << "\tPRIO: " << msg.payload.c_str() << std::endl;
  }
  void HandleIMessage(IMessage* message)
  {
    std::cout << "Default: " << std::to_string((uint32_t)message->type) << std::endl;
  }
  virtual void Dispatch(IMessage* message)
  {
    std::cout << "What is this even?...: " << std::to_string((uint32_t)message->type) << std::endl;
  }
};

void SendMessage()
{
  CListener custom1;
  CMessage message;
  message.payload = "That's a game!";

  ChannelMessageManager channel_message_manager;
  GlobalMessageManager global_message_manager;

  channel_message_manager.RegisterChannel("Prio");

  channel_message_manager.RegisterListener("Default", &custom1, [](IListener* listener, IMessage* message) { ((CListener*)listener)->HandleCMessage(message);     });
  channel_message_manager.RegisterListener("Default", &custom1, [](IListener* listener, IMessage* message) { ((CListener*)listener)->HandleIMessage(message);     });
  channel_message_manager.RegisterListener("Prio",    &custom1, [](IListener* listener, IMessage* message) { ((CListener*)listener)->HandleCMessagePRIO(message); });

  channel_message_manager.Dispatch("Default", &message);

  global_message_manager.RegisterListener(&custom1);
  global_message_manager.Dispatch(&message);
}





class TimeStampManager
{
public:
  TimeStampManager()
  {
    read();
  }
  void read()
  {
    if (!lambda::FileSystem::DoesFileExist("generated/timestamps"))
    {
      lambda::foundation::Error("Could not open file: generated/timestamps.\n");
      return;
    }
    lambda::Vector<char> data = lambda::FileSystem::FileToVector("generated/timestamps");
    uint32_t offset = 0u;
    while (offset < data.size())
    {
      lambda::Vector<char> time_stamp_vec(data.begin() + offset, std::find(data.begin() + offset, data.end(), ' '));
      lambda::String time_stamp(time_stamp_vec.size(), '\0');
      memcpy((void*)time_stamp.data(), time_stamp_vec.data(), time_stamp_vec.size());
      offset += (uint32_t)time_stamp.size() + 1u;
      lambda::Vector<char> file_name_vec(data.begin() + offset, std::find(data.begin() + offset, data.end(), '\n'));
      lambda::String file_name(file_name_vec.size(), '\0');
      memcpy((void*)file_name.data(), file_name_vec.data(), file_name_vec.size());
      offset += (uint32_t)file_name.size() + 1u;

      // Add the file name and time stamp.
      file_names_.push_back(file_name);
      time_stamps_.push_back(std::stoull(time_stamp.c_str()));
    }
  }
  void write()
  {
    // Get the size of the data.
    uint32_t size = 0u;
    for (uint32_t i = 0u; i < file_names_.size(); ++i)
    {
      size += (uint32_t)lambda::toString(time_stamps_[i]).size() + 1u;
      size += (uint32_t)file_names_[i].size() + 1u;
    }

    lambda::Vector<char> data(size);
    uint32_t offset = 0u;

    // Write the data.
    for (uint32_t i = 0u; i < file_names_.size(); ++i)
    {
      lambda::String time_stamp = lambda::toString(time_stamps_[i]);
      memcpy(data.data() + offset, time_stamp.data(), time_stamp.size());
      offset += (uint32_t)time_stamp.size();
      memset(data.data() + offset, ' ', 1u);
      offset += 1u;
      memcpy(data.data() + offset, file_names_[i].data(), file_names_[i].size());
      offset += (uint32_t)file_names_[i].size();
      memset(data.data() + offset, '\n', 1u);
      offset += 1u;
    }

    // Write the file.
    lambda::FileSystem::WriteFile("generated/timestamps", data);
  }

  bool hasFileChanged(const lambda::String& file_name) const
  {
    for (uint32_t i = 0u; i < file_names_.size(); ++i)
      if (file_name == file_names_[i])
        return hasFileChanged(i);
    return false;
  }
  bool hasFileChanged(const uint32_t index) const
  {
    return lambda::FileSystem::GetTimeStamp(file_names_[index]) != time_stamps_[index];
  }
  void updateTimeStap(const lambda::String& file_name)
  {
    for (uint32_t i = 0u; i < file_names_.size(); ++i)
    {
      if (file_name == file_names_[i])
      {
        updateTimeStap(i);
        return;
      }
    }
  }
  void updateTimeStap(const uint32_t index)
  {
    const uint64_t time_stamp = lambda::FileSystem::GetTimeStamp(file_names_[index]);
    if (time_stamp != time_stamps_[index])
    {
      time_stamps_[index] = time_stamp;
      write();
    }
  }
  const lambda::Vector<lambda::String>& getFiles() const
  {
    return file_names_;
  }
  bool hasFile(lambda::String file_name) const
  {
    return (eastl::find(file_names_.begin(), file_names_.end(), file_name) != file_names_.end());
  }
  void addFile(lambda::String file_name)
  {
    if (hasFile(file_name))
      return;

    file_names_.push_back(file_name);
    time_stamps_.push_back(0u);
    write();
  }
  void removeFile(lambda::String file_name)
  {
    if (!hasFile(file_name))
      return;

    for (uint32_t i = 0u; i < file_names_.size(); ++i)
    {
      if (file_names_[i] == file_name)
      {
        file_names_.erase(file_names_.begin() + i);
        time_stamps_.erase(time_stamps_.begin() + i);
        write();
        return;
      }
    }
  }

private:
  lambda::Vector<lambda::String> file_names_;
  lambda::Vector<uint64_t>       time_stamps_;
};

void removeFile(const lambda::String& file, lambda::VioletTextureCompiler texture_compiler, lambda::VioletWaveCompiler wave_compiler)
{
  lambda::String extension = lambda::FileSystem::GetExtension(file);
  if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "hdr")
  {
    texture_compiler.RemoveTexture(texture_compiler.GetHash(file));
    lambda::foundation::Info("[TEX] " + file + " removed!\n");
  }
  else if (extension == "wav")
  {
    wave_compiler.RemoveWave(wave_compiler.GetHash(file));
    lambda::foundation::Info("[WAV] " + file + " removed!\n");
  }
  else if (extension == "fx")
    Warning("[SHA] " + file + " removed!\n");
  else if (extension == "as")
    Warning("[AS-] " + file + " removed!\n");
  else if (extension == "wren")
    Warning("[WRE] " + file + " removed!\n");
  else if (extension == "glb")
    Warning("[MDL] " + file + " removed!\n");
  else if (extension == "ttf")
    Warning("[FNT] " + file + " removed!\n");
  else if (extension == "txt")
    Warning("[TXT] " + file + " removed!\n");
  else if (extension == "ini")
    Warning("[INI] " + file + " removed!\n");
  else
    Error("[---] " + file + " removed!\n");
}

void updateFile(const lambda::String& file, lambda::VioletTextureCompiler texture_compiler, lambda::VioletWaveCompiler wave_compiler)
{
  lambda::String extension = lambda::FileSystem::GetExtension(file);
  if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "hdr")
  {
    lambda::foundation::Info("[TEX] " + file + "\n");
    texture_compiler.RemoveTexture(texture_compiler.GetHash(file));
    lambda::foundation::Info("\tCompiling...\n");

    lambda::TextureCompileInfo compile_info{};
    compile_info.file = file;
    texture_compiler.Compile(compile_info);
    lambda::foundation::Info("\tCompiled!\n");
    texture_compiler.Save();
    lambda::foundation::Info("\tSaved!\n");
  }
  else if (extension == "wav")
  {
    lambda::foundation::Info("[WAV] " + file + "\n");
    wave_compiler.RemoveWave(wave_compiler.GetHash(file));
    lambda::foundation::Info("\tCompiling...\n");

    lambda::WaveCompileInfo compile_info{};
    compile_info.file = file;
    wave_compiler.Compile(compile_info);
    lambda::foundation::Info("\tCompiled!\n");

    wave_compiler.Save();
    lambda::foundation::Info("\tSaved!\n");
  }
  else if (extension == "fx")
    Warning("[SHA] " + file + " changed!\n");
  else if (extension == "as")
    Warning("[AS-] " + file + " changed!\n");
  else if (extension == "chai")
    Warning("[CHA] " + file + " changed!\n");
  else if (extension == "glb")
    Warning("[MDL] " + file + " changed!\n");
  else if (extension == "ttf")
    Warning("[FNT] " + file + " changed!\n");
  else if (extension == "txt")
    Warning("[TXT] " + file + " changed!\n");
  else if (extension == "ini")
    Warning("[INI] " + file + " changed!\n");
  else
    Error("[---] " + file + " changed!\n");
}

int main(int argc, char** argv)
{
  //SendMessage();
   
  if (argc == 1)
    LMB_ASSERT(false, "No project folder was speficied!");
  lambda::FileSystem::SetBaseDir(argv[1]);
  TimeStampManager time_stamp_manager;
  lambda::VioletTextureCompiler texture_compiler;
  lambda::VioletWaveCompiler wave_compiler;

  while (true)
  {
    auto function_start = std::chrono::high_resolution_clock::now();
    lambda::Vector<lambda::String> previous_files = time_stamp_manager.getFiles();

    for (lambda::String file : lambda::FileSystem::GetAllFilesInFolderRecursive("", ""))
    {
      // Don't track hidden files.
      if (lambda::FileSystem::GetExtension(file).empty())
        continue;

      // Make the file relative, so it can be used by the file system.
      file = lambda::FileSystem::MakeRelative(file);

      // If this file does not exist yet. Add it.
      if (!time_stamp_manager.hasFile(file))
        time_stamp_manager.addFile(file);

      // Erase this file from the to delete list.
      lambda::String* it = nullptr;
      while ((it = std::find(previous_files.begin(), previous_files.end(), file)) != previous_files.end())
        previous_files.erase(it);

      // Update the file.
      if (time_stamp_manager.hasFileChanged(file))
        updateFile(file, texture_compiler, wave_compiler);

      time_stamp_manager.updateTimeStap(file);
    }

    // Remove all deleted files.
    for (const lambda::String& file : previous_files)
    {
      time_stamp_manager.removeFile(file);
      removeFile(file, texture_compiler, wave_compiler);
    }

    // Sleep if we should.
    auto function_end = std::chrono::high_resolution_clock::now();
    auto sleep_point = function_start + std::chrono::seconds(1);
    if (function_end < sleep_point)
      std::this_thread::sleep_for(sleep_point - function_end);
  }
}
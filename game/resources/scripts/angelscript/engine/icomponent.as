class IComponent
{
  IComponent() { id = 0; }
  IComponent(const IComponent&in other)
  {
      id = other.id;
  }
  IComponent@ opAssign(const IComponent&in other)
  {
      id = other.id;
      return this;
  }

  void Destroy()
  {
      PrintLine("[ERROR] Destroy needs to be overwritten!");
  }
  void Initialize()
  {
      PrintLine("[ERROR] Initialize needs to be overwritten!");
  }

  uint64 GetId() const
  {
    return id;
  }
  private uint64 id = 0;
}
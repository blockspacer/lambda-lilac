#include <gui/gui.h>
#include <gui/gpu_driver.h>
#include <gui/file_system.h>
#include <gui/font_handler.h>

#include <Ultralight/platform/Platform.h>
#include <Ultralight/platform/Config.h>

namespace lambda
{
	namespace gui
	{
		extern void Create()
		{
			ultralight::Platform& platform = ultralight::Platform::instance();

			ultralight::Config config;
			config.face_winding         = ultralight::kFaceWinding_Clockwise; // CW in D3D, CCW in OGL
			config.device_scale_hint    = 1.0;                                // Set DPI to monitor DPI scale
			config.font_family_standard = "Arial";                            // Default font family


			//MyGPUDriver*  gpu_driver  = new MyGPUDriver();
			//MyFileSystem* file_system = new MyFileSystem();
			//MyFontLoader* font_loader = new MyFontLoader();

			platform.set_config(config);
			//platform.set_gpu_driver(gpu_driver);
			//platform.set_file_system(file_system);
			//platform.set_font_loader(font_loader);
		}
	}
}
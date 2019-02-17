#pragma once
#include <Ultralight/platform/FontLoader.h>
#include <containers/containers.h>

namespace lambda
{
  namespace gui
  {
		///////////////////////////////////////////////////////////////////////////
		///// MY FONT LOADER //////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		class MyFontLoader
			: public ultralight::FontLoader
		{
		public:
			MyFontLoader();
			virtual ~MyFontLoader();

			virtual ultralight::String16 fallback_font() const override;
			
			virtual ultralight::Ref<ultralight::Buffer> Load(
				const ultralight::String16& family, 
				int weight, 
				bool italic, 
				float size) override;

		protected:
			Map<uint64_t, ultralight::RefPtr<ultralight::Buffer>> fonts_;
		};

		extern void Create();
  }
}

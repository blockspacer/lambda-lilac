#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"
#include "platform/shader_pass.h"
#include "utils/angle.h"
#include <glm/gtc/matrix_transform.hpp>
#include "platform/culling.h"

namespace lambda
{
	namespace components
	{
		enum class ShadowType
		{
			kNone,
			kDynamic,
			kGenerateOnce,
			kGenerated,
		};

		enum class LightType : unsigned char
		{
			kDirectional = 0,
			kPoint = 1,
			kSpot = 2,
			kCascade = 3,
			kUnknown = 255
		};

		class LightComponent : public IComponent
		{
		public:
			LightComponent(const entity::Entity& entity, scene::Scene& scene);
			LightComponent(const LightComponent& other);
			LightComponent();

			void setShadowType(const ShadowType& shadow_type);
			ShadowType getShadowType() const;
			void setColour(const glm::vec3& colour);
			glm::vec3 getColour() const;
			void setAmbient(const glm::vec3& ambient);
			glm::vec3 getAmbient() const;
			void setIntensity(const float& intensity);
			float getIntensity() const;
			void setShadowMapSizePx(uint32_t shadow_map_size_px);
			uint32_t getShadowMapSizePx() const;
			void setTexture(asset::VioletTextureHandle texture);
			asset::VioletTextureHandle getTexture() const;
			void setDepth(const float& depth);
			float getDepth() const;
			void setEnabled(const bool& enabled);
			bool getEnabled() const;
			void setDynamicFrequency(const uint8_t& frequency);
			uint8_t getDynamicFrequency() const;
			void setLightType(LightType type);
			LightType getLightType() const;
			void setRSM(bool rsm);
			bool getRSM() const;
			void setSize(const float& size);
			float getSize() const;
			void setCutOff(const utilities::Angle& cut_off);
			utilities::Angle getCutOff() const;
			void setOuterCutOff(const utilities::Angle& outer_cut_off);
			utilities::Angle getOuterCutOff() const;


		protected:
			scene::Scene* scene_;
		};

		namespace LightSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				LightType type = LightType::kUnknown;
				ShadowType shadow_type = ShadowType::kNone;
				glm::vec3 colour = glm::vec3(1.0f);
				glm::vec3 ambient = glm::vec3(0.0f);
				float intensity = 1.0f;
				asset::VioletTextureHandle texture;
				Vector<utilities::Culler> culler;
				entity::Entity    entity;
				utilities::Angle cut_off = utilities::Angle::fromDeg(80.0f);
				utilities::Angle outer_cut_off = utilities::Angle::fromDeg(90.0f);
				float size = 50.0f;
				bool enabled = true;
				bool rsm = false;
				uint8_t dynamic_frequency = 3u;
				uint8_t dynamic_index = 254u;
				bool valid = true;

				Vector<float>       depth;
				Vector<glm::mat4x4> projection;
				Vector<glm::mat4x4> view;
				Vector<glm::vec3>   view_position;

				// Shadow maps.
				uint32_t shadow_map_size_px = 1024u;
				Vector<asset::VioletTextureHandle> render_target_texture;
				Vector<asset::VioletTextureHandle> depth_target_texture;
				Vector<platform::RenderTarget> render_target;
				Vector<platform::RenderTarget> depth_target;
			};

			struct SystemData
			{
				Vector<Data>                  data;
				Map<entity::Entity, uint32_t> entity_to_data;
				Map<uint32_t, entity::Entity> data_to_entity;
				Set<entity::Entity>           marked_for_delete;
				Queue<uint32_t>               unused_data_entries;

				Data& add(const entity::Entity& entity);
				Data& get(const entity::Entity& entity);
				void  remove(const entity::Entity& entity);
				bool  has(const entity::Entity& entity);

				asset::VioletMeshHandle full_screen_mesh;

				String   shader_generate;
				String   shader_modify;
				uint32_t shader_modify_count;
				String   shader_publish;
				String   shader_shadow_type;

				platform::RenderTarget     default_shadow_map;
				asset::VioletTextureHandle default_texture;
			};

			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			LightComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			LightComponent addDirectionalLight(const entity::Entity& entity, scene::Scene& scene);
			LightComponent addPointLight(const entity::Entity& entity, scene::Scene& scene);
			LightComponent addSpotLight(const entity::Entity& entity, scene::Scene& scene);
			LightComponent addCascadedLight(const entity::Entity& entity, scene::Scene& scene);

			LightComponent getComponent(const entity::Entity& entity, scene::Scene& scene);

			/*
			* Generate: [Input] Nothing [Output] Shadow map.
			* Modify:   [Input] Shadow map [Output] Shadow map.
		* Modify Count: The amount of time the modify shader needs to be applied.
			* Publish:  [Input] Shadow map | Position | Normal | Metallic_Roughness [Output] Light map.
		* Shadow Type: The type of shadows that should be used.
			*/
			void setShaders(String generate, String modify, uint32_t modify_count, String publish, String shadow_type, scene::Scene& scene);

			void initialize(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);
			void onRender(scene::Scene& scene);
			void collectGarbage(scene::Scene& scene);

			void setColour(const entity::Entity& entity, const glm::vec3& colour, scene::Scene& scene);
			glm::vec3 getColour(const entity::Entity& entity, scene::Scene& scene);
			void setAmbient(const entity::Entity& entity, const glm::vec3& ambient, scene::Scene& scene);
			glm::vec3 getAmbient(const entity::Entity& entity, scene::Scene& scene);
			void setIntensity(const entity::Entity& entity, const float& intensity, scene::Scene& scene);
			float getIntensity(const entity::Entity& entity, scene::Scene& scene);
			void setShadowType(const entity::Entity& entity, const ShadowType& shadow_type, scene::Scene& scene);
			ShadowType getShadowType(const entity::Entity& entity, scene::Scene& scene);
			void setShadowMapSizePx(const entity::Entity& entity, uint32_t shadow_map_size_px, scene::Scene& scene);
			uint32_t getShadowMapSizePx(const entity::Entity& entity, scene::Scene& scene);
			void setCutOff(const entity::Entity& entity, const utilities::Angle& cut_off, scene::Scene& scene);
			utilities::Angle getCutOff(const entity::Entity& entity, scene::Scene& scene);
			void setOuterCutOff(const entity::Entity& entity, const utilities::Angle& outer_cut_off, scene::Scene& scene);
			utilities::Angle getOuterCutOff(const entity::Entity& entity, scene::Scene& scene);
			void setDepth(const entity::Entity& entity, const float& depth, scene::Scene& scene);
			float getDepth(const entity::Entity& entity, scene::Scene& scene);
			void setSize(const entity::Entity& entity, const float& size, scene::Scene& scene);
			float getSize(const entity::Entity& entity, scene::Scene& scene);
			void setTexture(const entity::Entity& entity, asset::VioletTextureHandle texture, scene::Scene& scene);
			asset::VioletTextureHandle getTexture(const entity::Entity& entity, scene::Scene& scene);
			void setEnabled(const entity::Entity& entity, const bool& enabled, scene::Scene& scene);
			bool getEnabled(const entity::Entity& entity, scene::Scene& scene);
			void setDynamicFrequency(const entity::Entity& entity, const uint8_t& frequency, scene::Scene& scene);
			uint8_t getDynamicFrequency(const entity::Entity& entity, scene::Scene& scene);
			void setRSM(const entity::Entity& entity, bool rsm, scene::Scene& scene);
			bool getRSM(const entity::Entity& entity, scene::Scene& scene);
			void setLightType(const entity::Entity& entity, LightType type, scene::Scene& scene);
			LightType getLightType(const entity::Entity& entity, scene::Scene& scene);

			void createShadowMaps(const entity::Entity& entity, scene::Scene& scene);

			void renderDirectional(const entity::Entity& entity, scene::Scene& scene);
			void renderSpot(const entity::Entity& entity, scene::Scene& scene);
			void renderPoint(const entity::Entity& entity, scene::Scene& scene);
			void renderCascade(const entity::Entity& entity, scene::Scene& scene);
		}
	}
}
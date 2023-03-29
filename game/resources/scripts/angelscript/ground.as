
enum GroundEnabledType
{
  kEnabled,
  kZeroHeight,
  kDisabled
}

class Ground
{
  private GroundEnabledType enabled = kEnabled;
  private Entity ground;
  private Entity water;
  private Asset::Mesh mesh;
  private Asset::Texture albedo_texture;
  private Asset::Texture normal_texture;
  private Asset::Texture mr_texture;

  private Vec3   offset;
  private Vec3   scale  = Vec3(5.0f, 50.0f, 5.0f);
  private uint32 width  = 250;
  private uint32 height = 250;
  private float water_height = -10.0f;
  private Array<float> heights;

  void Disable()
  {
    enabled = kDisabled;
  }
  void ZeroHeight()
  {
    enabled = kZeroHeight;
  }
  void Enable()
  {
    enabled = kEnabled;
  }
  float GetWaterHeight() const
  {
    if(enabled != kEnabled) return 0.0f;
    return water_height;
  }
  private float HeightOnTile(Vec2 tile) const
  {
    uint32 idx = uint32(tile.x) + uint32(tile.y) * uint32(width);
    if(idx < 0 || idx >= heights.Size())
    {
      return 0.0f;
    }
    return heights[idx];
  }

  float HeightOnPosition(Vec3 position) const
  {
    if(enabled != kEnabled) return 0.0f;

    Vec3 t = (position - offset) / scale;
    Vec2 tile(t.x, t.z);

    if(tile.x < 0 || tile.y < 0 || tile.x >= float(width) || tile.y >= float(height))
    {
      return 0.0f;
    }
    else
    {
      float bl = HeightOnTile(Vec2(Floor(tile.x), Floor(tile.y)));
      float tl = HeightOnTile(Vec2(Floor(tile.x), Floor(tile.y) + 1));
      float br = HeightOnTile(Vec2(Floor(tile.x) + 1, Floor(tile.y)));
      float tr = HeightOnTile(Vec2(Floor(tile.x) + 1, Floor(tile.y) + 1));

      float a = Lerp(bl, br, tile.x - Floor(tile.x));
      float b = Lerp(tl, tr, tile.x - Floor(tile.x));
      float c = Lerp(a,  b,  tile.y - Floor(tile.y));
      return c;
    }
  }

  private Vec3 NormalOfQuad(Vec3 a, Vec3 b, Vec3 c, Vec3 d) const
  {
    Vec3 normal;

    normal += a.Cross(b); 
    normal += b.Cross(c); 
    normal += c.Cross(d); 
    normal += d.Cross(a); 
    
    return normal.Normalized();
  }
  void Initialize()
  {
    if (enabled == kDisabled) return;
    
    mesh = asset_manager.CreateMesh();
    albedo_texture = asset_manager.LoadTexture("resources/textures/mossy-ground1-albedo.png");
    normal_texture = asset_manager.LoadTexture("resources/textures/mossy-ground1-normal.png");

    Noise noise_big;
    noise_big.SetFrequency(0.01f);
    noise_big.SetSeed(256);
    noise_big.SetInterp(NoiseInterpolation::kQuintic);
    
    Noise noise_small;
    noise_small.SetFrequency(0.1f);
    noise_small.SetSeed(noise_big.GetSeed() * 2);
    noise_small.SetInterp(NoiseInterpolation::kQuintic);

    // Generate positions;
    Array<Vec2> tex_coords;
    Array<Vec3> normals;
    Array<Vec3> positions;
    for(uint32 h = 0; h < height; ++h)
    {
      for(uint32 w = 0; w < width; ++w)
      {
        Vec2 position(float(w), float(h));
        float new_height = noise_big.GetPerlin(position * Vec2(scale.x, scale.z));
        new_height += noise_small.GetPerlin(position * Vec2(scale.x, scale.z)) / scale.y;
        positions.PushBack(Vec3(position.x, new_height, position.y) * scale);
        if (enabled == kZeroHeight) positions[positions.Size() - 1].y = 0.0f;
        heights.PushBack(positions[positions.Size() - 1].y);

        tex_coords.PushBack(Vec2(float(w) % 2.0f, float(h) % 2.0f));
      }
    }

    Array<uint32> indices;
    normals.Resize(positions.Size());
    indices.Resize(positions.Size() * 6);

    for(uint32 h = 0; h < height - 1; ++h)
    {
      for(uint32 w = 0; w < width - 1; ++w)
      {
        uint32 idx_a = (h + 0) * width + (w + 0);
        uint32 idx_b = (h + 0) * width + (w + 1);
        uint32 idx_c = (h + 1) * width + (w + 0);
        uint32 idx_d = (h + 1) * width + (w + 1);

        Vec3 a = positions[idx_a];
        Vec3 b = positions[idx_b];
        Vec3 c = positions[idx_c];
        Vec3 d = positions[idx_d];

        normals[idx_a] = -(d - a).Cross(c - b);

        indices[idx_a * 6 + 0] = idx_c;
        indices[idx_a * 6 + 1] = idx_b;
        indices[idx_a * 6 + 2] = idx_a;
        indices[idx_a * 6 + 3] = idx_c;
        indices[idx_a * 6 + 4] = idx_d;
        indices[idx_a * 6 + 5] = idx_b;
      }
    }
    
    for(uint32 h = 0; h < height; ++h)
    {
        normals[(h) * width + (width - 1)] = Vec3(0.0f, 1.0f, 0.0f);
    }
    for(uint32 w = 0; w < width; ++w)
    {
        normals[(height - 1) * width + (w)] = Vec3(0.0f, 1.0f, 0.0f);
    }

    mesh.SetPositions(positions);
    mesh.SetNormals(normals);
    mesh.SetTexCoords(tex_coords);
    mesh.SetIndices(indices);
    mesh.RecalculateTangents();

    offset = -Vec3(float(width) * 0.5f, 0.0f, float(height) * 0.5f) * scale;
    
    ground.Create();
    Transform@ transform;
    MeshRender@ mesh_render;
    ground.AddComponent(@transform);
    ground.AddComponent(@mesh_render);
    mesh_render.SetMesh(mesh);
    mesh_render.SetAlbedoTexture(albedo_texture);
    mesh_render.SetNormalTexture(normal_texture);
    transform.SetLocalPosition(offset);
    mesh_render.MakeStatic();

    MakeMeshCollider(ground.GetId(), mesh);

    CreateWater();
  }

  private void CreateWater()
  {
    if(enabled != kEnabled) return;

    Asset::Mesh water_mesh = asset_manager.CreateMesh();
    Array<Vec3> water_positions;
    Array<Vec3> water_normals;
    Array<Vec2> water_tex_coords;
    Array<uint32> water_indices;

    Vec2 size = Vec2(float(width), float(height)) * Vec2(scale.x, scale.z);

    water_positions.PushBack(Vec3(0.0f, water_height, size.y));
    water_positions.PushBack(Vec3(size.x, water_height, size.y));
    water_positions.PushBack(Vec3(size.x, water_height, 0.0f));
    water_positions.PushBack(Vec3(0.0f, water_height, 0.0f));
    
    water_normals.PushBack(Vec3(0.0f, 1.0f, 0.0f));
    water_normals.PushBack(Vec3(0.0f, 1.0f, 0.0f));
    water_normals.PushBack(Vec3(0.0f, 1.0f, 0.0f));
    water_normals.PushBack(Vec3(0.0f, 1.0f, 0.0f));

    water_tex_coords.PushBack(Vec2(0.0f, 0.0f));
    water_tex_coords.PushBack(Vec2(size.x, 0.0f));
    water_tex_coords.PushBack(Vec2(size.x, size.y));
    water_tex_coords.PushBack(Vec2(0.0f, size.y));

    water_indices.PushBack(0);
    water_indices.PushBack(1);
    water_indices.PushBack(3);
    water_indices.PushBack(1);
    water_indices.PushBack(2);
    water_indices.PushBack(3);

    water_mesh.SetPositions(water_positions);
    water_mesh.SetNormals(water_normals);
    water_mesh.SetTexCoords(water_tex_coords);
    water_mesh.SetIndices(water_indices);
    water_mesh.RecalculateTangents();

    
    albedo_texture = asset_manager.LoadTexture("resources/textures/Water-0326-diffuse.png");
    normal_texture = asset_manager.LoadTexture("resources/textures/Water-0326-normal.png");

    water.Create();
    Transform@ transform;
    MeshRender@ mesh_render;
    water.AddComponent(@transform);
    water.AddComponent(@mesh_render);
    mesh_render.SetMesh(water_mesh);
    mesh_render.SetAlbedoTexture(albedo_texture);
    mesh_render.SetNormalTexture(normal_texture);
    transform.SetLocalPosition(offset);
    mesh_render.MakeStatic();
  }
};
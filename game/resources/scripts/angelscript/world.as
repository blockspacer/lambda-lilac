class World
{
  void Disable()
  {
    enabled = false;
  }
  void Enable()
  {
    enabled = true;
  }

  void AddGeneratedModel(const String&in type)
  {
    if(!enabled) return;

    Asset::Mesh mesh = asset_manager.GenerateMesh(type);

    Entity model;
    model.Create();

    Transform@ transform;
    MeshRender@ mesh_render;
    model.AddComponent(@transform);
    model.AddComponent(@mesh_render);
    mesh_render.Attach(mesh);
    MakeMeshCollider(model.GetId(), mesh);
    mesh_render.MakeStaticRecursive();
    
    models.PushBack(model);
  }
  void AddModel(const String&in file)
  {
    if(!enabled) return;

    Asset::Mesh mesh = asset_manager.LoadMesh(file);

    Entity model;
    model.Create();

    Transform@ transform;
    MeshRender@ mesh_render;
    model.AddComponent(@transform);
    model.AddComponent(@mesh_render);
    mesh_render.Attach(mesh);
    transform.SetLocalPosition(Vec3(0.0f, 5.5f, 0.0f));
    //MakeMeshCollider(model.GetId(), mesh);
    mesh_render.MakeStaticRecursive();
    
    models.PushBack(model);
  }

  private bool enabled = true;
  private Array<Entity> models;
}
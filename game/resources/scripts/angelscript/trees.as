#include "ground.as"

class Trees
{
  void Disable()
  {
    enabled = false;
  }
  void Enable()
  {
    enabled = true;
  }

  void Initialize(const Ground&in ground)
  {
    if(!enabled) return;

    // Spawn all of the trees.
    Asset::Mesh tree_mesh_01 = asset_manager.LoadMesh("resources/gltf/tree_01.glb");
    Asset::Mesh tree_mesh_02 = asset_manager.LoadMesh("resources/gltf/tree_02.glb");
    Asset::Mesh tree_mesh_03 = asset_manager.LoadMesh("resources/gltf/tree_03.glb");
    Asset::Mesh tree_mesh_04 = asset_manager.LoadMesh("resources/gltf/tree_04.glb");
    
    Asset::Mesh bush_mesh_01 = asset_manager.LoadMesh("resources/gltf/bush_01.glb");
    Asset::Mesh bush_mesh_02 = asset_manager.LoadMesh("resources/gltf/bush_02.glb");
    Asset::Mesh bush_mesh_03 = asset_manager.LoadMesh("resources/gltf/bush_03.glb");

    const float scale = 10.0f;
    const float rarity = 0.33f;
    const int16 num_trees = 5000;
    const int16 num_x = int16(Sqrt(float(num_trees))) - 3;
    const int16 num_z = num_trees / num_x;

    int16 actual_count = 0;

    for(int x = -num_x / 2; x <= num_x / 2; ++x)
    {
      if(x != 0 && x != -1)
      {
        for(int z = -num_z / 2; z <= num_z / 2; ++z)
        {
          Vec3 position = Vec3(
            (x + Random(0.2f, 0.8f)) * scale,
            0.0f,
            (z + Random(0.2f, 0.8f)) * scale
          );
          
          position.y = ground.HeightOnPosition(position);

          if (Random() < rarity && position.y > ground.GetWaterHeight())
          {
            actual_count++;
            Entity tree;
            tree.Create();
            Transform@ transform;
            MeshRender@ mesh_render;
            tree.AddComponent(@transform);
            tree.AddComponent(@mesh_render);
            
            transform.SetLocalPosition(position);
            int8 selected_mesh = int8(Random() * (4.0f * 2.0f + 3.0f));
            selected_mesh = (selected_mesh < 8) ? (selected_mesh / 2) : (selected_mesh - 4);

            switch(selected_mesh)
            {
            case 0:
              mesh_render.Attach(tree_mesh_01);
              MakeMeshCollider(tree.GetId(), tree_mesh_01);
              break;
            case 1:
              mesh_render.Attach(tree_mesh_02);
              MakeMeshCollider(tree.GetId(), tree_mesh_02);
              break;
            case 2:
              mesh_render.Attach(tree_mesh_03);
              MakeMeshCollider(tree.GetId(), tree_mesh_03);
              break;
            case 3:
              mesh_render.Attach(tree_mesh_04);
              MakeMeshCollider(tree.GetId(), tree_mesh_04);
              break;
            case 4:
              mesh_render.Attach(bush_mesh_01);
              MakeMeshCollider(tree.GetId(), bush_mesh_01);
              break;
            case 5:
              mesh_render.Attach(bush_mesh_02);
              MakeMeshCollider(tree.GetId(), bush_mesh_02);
              break;
            case 6:
              mesh_render.Attach(bush_mesh_03);
              MakeMeshCollider(tree.GetId(), bush_mesh_03);
              break;
            }
            mesh_render.MakeStaticRecursive();
            trees.PushBack(tree);
          } 
        }
      }
    }

    Info("[TREE]: Hard maximum number of trees: "      + num_trees);
    Info("[TREE]: Estimated maximum number of trees: " + num_x * num_z);
    Info("[TREE]: Actual number of trees: "            + actual_count);
  }

  private bool enabled = true;
  private Array<Entity> trees;
}
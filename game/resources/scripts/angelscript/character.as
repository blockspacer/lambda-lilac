#include "engine/asset_manager.as"
#include "engine/graphics.as"

class Character
{
    void Create()
    {
        // Load the models.
        Asset::Mesh mesh_sphere = asset_manager.GenerateMesh("sphere");
        Asset::Mesh mesh_cube   = asset_manager.GenerateMesh("cube");
        
        // Create all entities.
        m_root           = CreateEntity();
        m_rootMesh       = CreateEntity();
        m_shoulder_left  = CreateEntity();
        m_shoulder_right = CreateEntity();
        m_hand_left      = CreateEntity();
        m_hand_right     = CreateEntity();
        m_handMesh_left  = CreateEntity();
        m_handMesh_right = CreateEntity();
        m_pelvis         = CreateEntity();
        m_thigh_left     = CreateEntity();
        m_thigh_right    = CreateEntity();
        m_foot_left      = CreateEntity();
        m_foot_right     = CreateEntity();
        m_footMesh_left  = CreateEntity();
        m_footMesh_right = CreateEntity();

        // Set the parenting tree.
        SetParent(m_root,           m_rootMesh);
        SetParent(m_root,           m_shoulder_left);
        SetParent(m_shoulder_left,  m_hand_left);
        SetParent(m_hand_left,      m_handMesh_left);
        SetParent(m_root,           m_shoulder_right);
        SetParent(m_shoulder_right, m_hand_right);
        SetParent(m_hand_right,     m_handMesh_right);
        SetParent(m_root,           m_pelvis);
        SetParent(m_pelvis,         m_thigh_left);
        SetParent(m_thigh_left,     m_foot_left);
        SetParent(m_foot_left,      m_footMesh_left);
        SetParent(m_pelvis,         m_thigh_right);
        SetParent(m_thigh_right,    m_foot_right);
        SetParent(m_foot_right,     m_footMesh_right);

        // Initialize all entities.
        InitializeEntity(m_root,           Vec3(+0.0f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_rootMesh,       Vec3(+0.0f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f), mesh_sphere);
        InitializeEntity(m_shoulder_left,  Vec3(-0.5f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_shoulder_right, Vec3(+0.5f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_hand_left,      Vec3(-1.0f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_hand_right,     Vec3(+1.0f,  +0.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_handMesh_left,  Vec3(-1.0f,  +0.0f, +0.0f),  Vec3(0.25f, 0.25f, 0.25f), mesh_cube);
        InitializeEntity(m_handMesh_right, Vec3(+1.0f,  +0.0f, +0.0f),  Vec3(0.25f, 0.25f, 0.25f), mesh_cube);
        InitializeEntity(m_pelvis,         Vec3(+0.0f,  -0.5f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_thigh_left,     Vec3(-0.33f, -0.5f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_thigh_right,    Vec3(+0.33f, -0.5f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_foot_left,      Vec3(-0.33f, -1.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_foot_right,     Vec3(+0.33f, -1.0f, +0.0f),  Vec3(1.0f,  1.0f,  1.0f));
        InitializeEntity(m_footMesh_left,  Vec3(-0.33f, -1.0f, +0.0f),  Vec3(0.25f, 0.15f, 0.5f), mesh_cube);
        InitializeEntity(m_footMesh_right, Vec3(+0.33f, -1.0f, +0.0f),  Vec3(0.25f, 0.15f, 0.5f), mesh_cube);
    }

    private Entity CreateEntity()
    {
        // Create the entity.
        Entity entity;
        entity.Create();

        // Attach a transform.
        Transform@ transform;
        entity.AddComponent(@transform);

        return entity;
    }
    private void SetParent(Entity parent, Entity child)
    {
        Transform@ transform_parent;
        Transform@ transform_child;
        parent.GetComponent(@transform_parent);
        child.GetComponent(@transform_child);
        transform_child.SetParent(transform_parent);
    }
    private void InitializeEntity(Entity entity, const Vec3&in position, const Vec3&in scale)
    {
        // Get the transform.
        Transform@ transform;
        entity.GetComponent(@transform);

        // Set the position and scale.
        transform.SetWorldPosition(position);
        transform.SetWorldScale(scale);
    }
    private void InitializeEntity(Entity entity, const Vec3&in position, const Vec3&in scale, const Asset::Mesh&in mesh)
    {
        // Initialize all other variables using the already existing initialize function.
        InitializeEntity(entity, position, scale);
        
        // Attach the mesh renderer and set the mesh.
        MeshRender@ mesh_render;
        entity.AddComponent(@mesh_render);
        mesh_render.SetMesh(mesh);
        mesh_render.SetSubMesh(0);
    }

    void Update(const float delta_time)
    {
        m_time += delta_time;

        Transform@ transform;
        m_shoulder_left.GetComponent(@transform);
        transform.SetWorldRotationEuler(Vec3(0.0f, 90.0f, m_time * 10.0f));
    }

    private float  m_time;
    private Entity m_root;
    private Entity m_rootMesh;
    private Entity m_shoulder_left;
    private Entity m_shoulder_right;
    private Entity m_hand_left;
    private Entity m_hand_right;
    private Entity m_handMesh_left;
    private Entity m_handMesh_right;
    private Entity m_pelvis;
    private Entity m_thigh_left;
    private Entity m_thigh_right;
    private Entity m_foot_left;
    private Entity m_foot_right;
    private Entity m_footMesh_left;
    private Entity m_footMesh_right;


    //      1---0---1
    //     /    |    \
    //    2     |     2
    //        4-3-4
    //        |   |
    //        |   |
    //        5   5
    // 0: Root
    // 1: Shoulders
    // 2: Hands
    // 3: Pelvis
    // 4: Thighs
    // 5: Feet
}
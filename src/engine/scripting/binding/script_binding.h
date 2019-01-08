namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    extern void ScriptBinding(world::IWorld* world);
    extern void ScriptRelease();
  }
}
import "resources/scripts/wren/demos/three" for Demo
import "Core" for GUI, Graphics, Physics

class World {
  construct new() {
    _demo = Demo.new()
  }

  initialize() {
    GUI.bindCallback("changedSetting(_,_)", this)
    GUI.bindCallback("movedToMenu(_)", this)
    _demo.initialize()
  }
  deinitialize() {
    _demo.deinitialize()
  }
  update() {
    _demo.update()
  }

  fixedUpdate() {
    _demo.fixedUpdate()
  }

  changedSetting(id, val) {
    if (id == "vsync") {
      Graphics.vsync = val > 0.0 ? true : false
    }
    if (id == "render_scale") {
      Graphics.renderScale = val
    }
    if (id == "physics_debug_draw") {
      Physics.debugDrawEnabled = val > 0.0 ? true : false
    }
  }

  movedToMenu(menu) {
    // TODO (Hilze): Implement.
    //_camera.getComponent(ThirdPersonCamera).hasInput = menu == "" ? true : false
  }
}

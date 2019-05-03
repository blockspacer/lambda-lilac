import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math
import "Core/Graphics" for Graphics
import "Core/Vec2" for Vec2

class InputController {
    static MovementHorizontal {
        var val = 0.0
        //val = Input.getAxis(Axes.LeftStickX)
        if (Input.getKey(Keys.A)) val = val - 1.0
        if (Input.getKey(Keys.D)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementVertical {
        var val = 0.0
        //val = Input.getAxis(Axes.LeftStickY)
        if (Input.getKey(Keys.S)) val = val - 1.0
        if (Input.getKey(Keys.W)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static CameraHorizontal { // TODO (Hilze): Add mouse support.
        var val = 0.0
        //val = Input.getAxis(Axes.RightStickX)
        if (Input.getKey(Keys.Right)) val = val - 1.0
        if (Input.getKey(Keys.Left)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }
    
    static CameraAttack { // TODO (Hilze): Add mouse support.
        var val = 0.0
        if (Input.getButton(Buttons.LeftMouseButton)) val = val + 1.0
        if (Input.getKey(Keys.Q)) val = val + 1.0
        return Math.clamp(val, -1.0, 1.0)
    }
    
    static CameraVertical { // TODO (Hilze): Add mouse support.
        var val = 0.0
        //val = Input.getAxis(Axes.RightStickY)
        if (Input.getKey(Keys.Down)) val = val - 1.0
        if (Input.getKey(Keys.Up)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementSprint {
        var val = 0.0
        //val = Input.getAxis(Axes.RightTrigger)
        if (Input.getKey(Keys.Shift)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementUpDown {
        var val = 0.0
        //if (Input.getButton(Buttons.Y)) val = val - 1.0
        //if (Input.getButton(Buttons.A)) val = val + 1.0
        //if (Input.getKey(Keys.Q)) val = val - 1.0
        if (Input.getKey(Keys.Space)) val = val + 1.0
        if (Input.getKey(Keys.Shift)) val = val - 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static FreeMe {
        var val = 0.0
        //if (Input.getButton(Buttons.X)) val = val + 1.0
        if (Input.getKey(Keys.P)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static NodePlaceRemove {
        var val = 0.0
        if (Input.getKey(Keys.F)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static NodeConnect {
        var val = 0.0
        if (Input.getKey(Keys.G)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static ToggleNodeEditor {
        var val = 0.0
        if (Input.getKey(Keys.Tab)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MousePosition {
        var val = Vec2.new(Input.getAxis(Axes.MouseX), Input.getAxis(Axes.MouseY)) / Graphics.windowSize
        return (val * 2.0 - 1.0) * Vec2.new(1.0, -1.0)
    }
}
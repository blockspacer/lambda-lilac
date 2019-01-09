import "Core/Input" for Input, Keys, Buttons, Axes
import "Core/Math" for Math

class InputController {
    static MovementHorizontal {
        var val = 0.0
        val = Input.getAxis(Axes.LeftStickX)
        if (Input.getKey(Keys.A)) val = val - 1.0
        if (Input.getKey(Keys.D)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementVertical {
        var val = 0.0
        val = Input.getAxis(Axes.LeftStickY)
        if (Input.getKey(Keys.S)) val = val - 1.0
        if (Input.getKey(Keys.W)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static CameraHorizontal { // TODO (Hilze): Add mouse support.
        var val = 0.0
        val = Input.getAxis(Axes.RightStickX)
        if (Input.getKey(Keys.Right)) val = val - 1.0
        if (Input.getKey(Keys.Left)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }
    
    static CameraVertical { // TODO (Hilze): Add mouse support.
        var val = 0.0
        val = Input.getAxis(Axes.RightStickY)
        if (Input.getKey(Keys.Down)) val = val - 1.0
        if (Input.getKey(Keys.Up)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementSprint {
        var val = 0.0
        val = Input.getAxis(Axes.RightTrigger)
        if (Input.getKey(Keys.Shift)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static MovementUpDown {
        var val = 0.0
        if (Input.getButton(Buttons.Y)) val = val - 1.0
        if (Input.getButton(Buttons.A)) val = val + 1.0
        if (Input.getKey(Keys.Q)) val = val - 1.0
        if (Input.getKey(Keys.E)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }

    static FreeMe {
        var val = 0.0
        if (Input.getButton(Buttons.X)) val = val + 1.0
        if (Input.getKey(Keys.P)) val = val + 1.0

        return Math.clamp(val, -1.0, 1.0)
    }
}
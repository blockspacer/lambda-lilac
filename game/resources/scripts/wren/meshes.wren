import "Core" for Mesh

class Meshes {
    static cube {
        if (__cube == null) __cube = Mesh.generate("cube")
        return __cube
    }
    static sphere {
        if (__sphere == null) __sphere = Mesh.generate("sphere")
        return __sphere
    }
}
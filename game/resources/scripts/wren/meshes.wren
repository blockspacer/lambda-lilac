import "Core" for Mesh, Vec3

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

class MeshCreator {
  construct new () {
    _mesh = Mesh.create()
    clear()
  }

  
  addTriDS(posA, posB, posC, uvA, uvB, uvC) {
    addTri(posA, posB, posC, uvA, uvB, uvC)
    addTri(posC, posB, posA, uvC, uvB, uvA)
  }

  addTri(posA, posB, posC, uvA, uvB, uvC) {
   	var u = posB - posA
    var v = posC - posA
    var normal = Vec3.new(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x)

    var t1 = normal.cross(Vec3.new(1.0, 0.0, 0.0))
    var t2 = normal.cross(Vec3.new(0.0, 1.0, 0.0))
    var tangent = (t1.lengthSqr > t2.lengthSqr) ? t1 : t2

    _positions.add(posA)
    _positions.add(posB)
    _positions.add(posC)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _uvs.add(uvA)
    _uvs.add(uvB)
    _uvs.add(uvC)
    _indices.add(_positions.count - 3)
    _indices.add(_positions.count - 2)
    _indices.add(_positions.count - 1)
    _baseIndex = _baseIndex + 3
  }

  addQuad(posA, posB, posC, posD, uvA, uvB, uvC, uvD) {
   	var u = posB - posA
    var v = posC - posA
    var normal = Vec3.new(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x)

    var t1 = normal.cross(Vec3.new(1.0, 0.0, 0.0))
    var t2 = normal.cross(Vec3.new(0.0, 1.0, 0.0))
    var tangent = (t1.lengthSqr > t2.lengthSqr) ? t1 : t2

    _positions.add(posA)
    _positions.add(posB)
    _positions.add(posC)
    _positions.add(posD)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _uvs.add(uvA)
    _uvs.add(uvB)
    _uvs.add(uvC)
    _uvs.add(uvD)
    _indices.add(_baseIndex + 0)
    _indices.add(_baseIndex + 1)
    _indices.add(_baseIndex + 2)
    _indices.add(_baseIndex + 2)
    _indices.add(_baseIndex + 3)
    _indices.add(_baseIndex + 0)
    _baseIndex = _baseIndex + 4
  }

  clear() {
      _positions = []
      _normals   = []
      _tangents  = []
      _uvs       = []
      _indices   = []
      _baseIndex = 0
  }

  make {
    _mesh.positions = _positions
    _mesh.normals   = _normals
    _mesh.tangents  = _tangents
    _mesh.texCoords = _uvs
    _mesh.indices   = _indices
    return _mesh
  }
}

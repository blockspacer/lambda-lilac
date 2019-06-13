import "Core" for Mesh, Math, Vec2, Vec3

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

  addTriAuto(posA, posB, posC) {
    var up    = (posC - posA).normalized
    //up.x = Math.abs(up.x)
    //up.y = Math.abs(up.y)
    //up.z = Math.abs(up.z)
    var r1 = up.cross(Vec3.new(0.0, 1.0, 0.0))
    var r2 = up.cross(Vec3.new(0.0, 0.0, 1.0))
    var right = (r1.lengthSqr > r2.lengthSqr) ? r1 : r2
    right.normalize()
    //right.x = Math.abs(right.x)
    //right.y = Math.abs(right.y)
    //right.z = Math.abs(right.z)

    var dotA = Vec2.new(right.dot(posA), up.dot(posA))
    var dotB = Vec2.new(right.dot(posB), up.dot(posB))
    var dotC = Vec2.new(right.dot(posC), up.dot(posC))
    var min  = Vec2.new(Math.min(Math.min(dotA.x, dotB.x), dotC.x), Math.min(Math.min(dotA.y, dotB.y), dotC.y))
    dotA = dotA - min
    dotB = dotB - min
    dotC = dotC - min

    addTri(posA, posB, posC, dotA, dotB, dotC)
  }

  addTri(posA, posB, posC, uvA, uvB, uvC) {
   	var u = posB - posA
    var v = posC - posA
    var normal = Vec3.new(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x)

    var t1 = normal.cross(Vec3.new(1.0, 0.0, 0.0))
    var t2 = normal.cross(Vec3.new(0.0, 1.0, 0.0))
    var tangent = (t1.lengthSqr > t2.lengthSqr) ? t1 : t2

    _positions.add(Vec3.new(posA.x, posA.y, posA.z))
    _positions.add(Vec3.new(posB.x, posB.y, posB.z))
    _positions.add(Vec3.new(posC.x, posC.y, posC.z))
    _normals.add(normal)
    _normals.add(normal)
    _normals.add(normal)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _tangents.add(tangent)
    _uvs.add(Vec2.new(uvA.x, uvA.y))
    _uvs.add(Vec2.new(uvB.x, uvB.y))
    _uvs.add(Vec2.new(uvC.x, uvC.y))
    _indices.add(_positions.count - 3)
    _indices.add(_positions.count - 2)
    _indices.add(_positions.count - 1)
    _baseIndex = _baseIndex + 3
  }

  addQuadAuto(posA, posB, posC, posD) {
    var up    = (posC - posA).normalized
    var right = (posB - posA).normalized

    var dotA = Vec2.new(right.dot(posA), up.dot(posA))
    var dotB = Vec2.new(right.dot(posB), up.dot(posB))
    var dotC = Vec2.new(right.dot(posC), up.dot(posC))
    var dotD = Vec2.new(right.dot(posD), up.dot(posD))
    var min  = Vec2.new(Math.min(Math.min(Math.min(dotA.x, dotB.x), dotC.x), dotD.x), Math.min(Math.min(Math.min(dotA.y, dotB.y), dotC.y), dotD.y))
    dotA = dotA - min
    dotB = dotB - min
    dotC = dotC - min
    dotD = dotD - min

    addQuad(posA, posB, posC, posD, dotA, dotB, dotC, dotD)
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

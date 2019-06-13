static constexpr char* wrenModuleSource =
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// console /////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Console
* _*Console*_
*/
"class Console {\n"
/*
* Function: :debug(_)
* _*static*_ Outputs a string using the *debug* channel. Adds a *new line* at the end of the string.
*
* Parameters:
* string - The string that will be outputted to the console. Needs to be String
*/
"foreign static debug(string)\n"
/*
* Function: :error(_)
* _*static*_ Outputs a string using the *error* channel. Adds a *new line* at the end of the string.
*
* Parameters:
* string - The string that will be outputted to the console. Needs to be String
*/
"foreign static error(string)\n"
/*
* Function: :info(_)
* _*static*_ Outputs a string using the *info* channel. Adds a *new line* at the end of the string.
*
* Parameters:
* string - The string that will be outputted to the console. Needs to be String
*/
"foreign static info(string)\n"
/*
* Function: :warning(_)
* _*static*_ Outputs a string using the *warning* channel. Adds a *new line* at the end of the string.
*
* Parameters:
* string - The string that will be outputted to the console. Needs to be String
*/
"foreign static warning(string)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// vec2 ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Vec2
* _*Vector2*_
*/
"foreign class Vec2 {\n"
/*
* Constructor: :new()
* _*Constructor*_ Constructs a Vec2 intialized to X: 0.0f and Y: 0.0f.
*/
"construct new() {}\n"
/*
* Constructor: :new(_)
* _*Constructor*_ Constructs a Vec2 intialized to X: v and Y: v.
*
* Parameters:
* v - The value that both X and Y will be initialized to. Needs to be Num.
*/
"construct new(v) {}\n"
/*
* Constructor: :new(_,_)
* _*Constructor*_ Constructs a Vec2 intialized to X: x and Y: y.
*
* Parameters:
* x - The value that X will be initialized to. Needs to be Num.
* y - The value that Y will be initialized to. Needs to be Num.
*/
"construct new(x, y) {}\n"
/*
* Function: :normalize()
* Normalizes the current vector.
*/
"foreign normalize()\n"
/*
* Function: :dot(_)
* Get the dot product between the current vector and a provided vector.
*
* Parameters:
* v - The vector which the dot product will be done with. Needs to be Vec2
*/
"foreign dot(v)\n"
/*
* Function: :+(_)
* Add a value to the current value and return the result.
*
* Parameters:
* v - The value that will be added to this. Needs to be Num or Vec2
*/
"+(v) {\n"
"if (v is Num)  return Vec2.new(x + v, y + v)\n"
"if (v is Vec2) return Vec2.new(x + v.x, y + v.y)\n"
"}\n"
/*
* Function: :-(_)
* Subtract a value from the current value and return the result.
*
* Parameters:
* v - The value that will be subtracted from this. Needs to be Num or Vec2
*/
"-(v) {\n"
"if (v is Num)  return Vec2.new(x - v, y - v)\n"
"if (v is Vec2) return Vec2.new(x - v.x, y - v.y)\n"
"}\n"
/*
* Function: :*(_)
* Multiply a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be multiplied with. Needs to be Num or Vec2
*/
"*(v) {\n"
"if (v is Num)  return Vec2.new(x * v, y * v)\n"
"if (v is Vec2) return Vec2.new(x * v.x, y * v.y)\n"
"}\n"
/*
* Function: :/(_)
* Divide a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be divided with. Needs to be Num or Vec2
*/
"/(v) {\n"
"if (v is Num)  return Vec2.new(x / v, y / v)\n"
"if (v is Vec2) return Vec2.new(x / v.x, y / v.y)\n"
"}\n"
/*
* Function: :==(_)
* Check if this is equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec2
*/
"==(v) {\n"
"if (v is Num) return x == v && y == v\n"
"if (v is Vec2) return x == v.x && y == v.y\n"
"}\n"
/*
* Function: :!=(_)
* Check if this is not equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec2
*/
"!=(v) {\n"
"if (v is Num) return x != v || y != v\n"
"if (v is Vec2) return x != v.x || y != v.y\n"
"}\n"
/*
* Function: :normalized
* Get a normalized version of the current vector.
*/
"foreign normalized\n"
/*
* Function: :toString
* Get a stringified version of the current vector.
*/
"foreign toString\n"
/*
* Function: :x
* Get the x component of this vector.
*/
"foreign x\n"
/*
* Function: :y
* Get the y component of this vector.
*/
"foreign y\n"
/*
* Function: :magnitude
* Get the magnitude of this vector.
*/
"foreign magnitude\n"
/*
* Function: :magnitudeSqr
* Get the squared magnitude of this vector.
*/
"foreign magnitudeSqr\n"
/*
* Function: :length
* Get the length of this vector.
*/
"foreign length\n"
/*
* Function: :lengthSqr
* Get the squared length of this vector.
*/
"foreign lengthSqr\n"
/*
* Function: :x=(_)
* Set the x component of this vector. Needs to be Num
*/
"foreign x=(x)\n"
/*
* Function: :y=(_)
* Set the y component of this vector. Needs to be Num
*/
"foreign y=(y)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// vec3 ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Vec3
* _*Vector3*_
*/
"foreign class Vec3 {\n"
/*
* Constructor: :new()
* _*Constructor*_ Constructs a Vec3 intialized to X: 0.0f, Y: 0.0f and Z: 0.0f.
*/
"construct new() {}\n"
/*
* Constructor: :new(_)
* _*Constructor*_ Constructs a Vec3 intialized to X: v, Y: v and Z: v.
*
* Parameters:
* v - The value that both X, Y and Z will be initialized to. Needs to be a Num.
*/
"construct new(v) {}\n"
/*
* Constructor: :new(_,_)
* _*Constructor*_ Constructs a Vec3 intialized to X: x, Y: y and Z: z.
*
* Parameters:
* x - The value that X will be initialized to. Needs to be Num.
* y - The value that Y will be initialized to. Needs to be Num.
* z - The value that Z will be initialized to. Needs to be Num.
*/
"construct new(x, y, z) {}\n"
/*
* Function: :normalize()
* Normalizes the current vector.
*/
"foreign normalize()\n"
/*
* Function: :dot(_)
* Get the dot product between the current vector and a provided vector.
*
* Parameters:
* v - The vector which the dot product will be done with. Needs to be Vec3
*/
"foreign dot(v)\n"
/*
* Function: :cross(_)
* Get the cross product between the current vector and a provided vector.
*
* Parameters:
* v - The vector which the cross product will be done with. Needs to be Vec3
*/
"foreign cross(v)\n"
/*
* Function: :+(_)
* Add a value to the current value and return the result.
*
* Parameters:
* v - The value that will be added to this. Needs to be Num or Vec3
*/
"+(v) {\n"
"if (v is Num)  return Vec3.new(x + v, y + v, z + v)\n"
"if (v is Vec3) return Vec3.new(x + v.x, y + v.y, z + v.z)\n"
"}\n"
/*
* Function: :-(_)
* Subtract a value from the current value and return the result.
*
* Parameters:
* v - The value that will be subtracted from this. Needs to be Num or Vec3
*/
"-(v) {\n"
"if (v is Num)  return Vec3.new(x - v, y - v, z - v)\n"
"if (v is Vec3) return Vec3.new(x - v.x, y - v.y, z - v.z)\n"
"}\n"
/*
* Function: :*(_)
* Multiply a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be multiplied with. Needs to be Num or Vec3
*/
"*(v) {\n"
"if (v is Num)  return Vec3.new(x * v, y * v, z * v)\n"
"if (v is Vec3) return Vec3.new(x * v.x, y * v.y, z * v.z)\n"
"}\n"
/*
* Function: :/(_)
* Divide a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be divided with. Needs to be Num or Vec3
*/
"/(v) {\n"
"if (v is Num)  return Vec3.new(x / v, y / v, z / v)\n"
"if (v is Vec3) return Vec3.new(x / v.x, y / v.y, z / v.z)\n"
"}\n"
/*
* Function: :==(_)
* Check if this is equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec3
*/
"==(v) {\n"
"if (v is Num) return x == v && y == v && z == v\n"
"if (v is Vec3) return x == v.x && y == v.y && z == v.z\n"
"}\n"
/*
* Function: :!=(_)
* Check if this is not equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec3
*/
"!=(v) {\n"
"if (v is Num) return x != v || y != v || z != v\n"
"if (v is Vec3) return x != v.x || y != v.y || z != v.z\n"
"}\n"
/*
* Function: :normalized
* Get a normalized version of the current vector.
*/
"foreign normalized\n"
/*
* Function: :toString
* Get a stringified version of the current vector.
*/
"foreign toString\n"
/*
* Function: :toString
* _*Static*_ Get a Vec3 from a string.
*/
"foreign static fromString(string)\n"
/*
* Function: :x
* Get the x component of this vector.
*/
"foreign x\n"
/*
* Function: :y
* Get the y component of this vector.
*/
"foreign y\n"
/*
* Function: :z
* Get the z component of this vector.
*/
"foreign z\n"
/*
* Function: :magnitude
* Get the magnitude of this vector.
*/
"foreign magnitude\n"
/*
* Function: :magnitudeSqr
* Get the squared magnitude of this vector.
*/
"foreign magnitudeSqr\n"
/*
* Function: :length
* Get the length of this vector.
*/
"foreign length\n"
/*
* Function: :lengthSqr
* Get the squared length of this vector.
*/
"foreign lengthSqr\n"
/*
* Function: :x=(_)
* Set the x component of this vector. Needs to be Num
*/
"foreign x=(x)\n"
/*
* Function: :y=(_)
* Set the y component of this vector. Needs to be Num
*/
"foreign y=(y)\n"
/*
* Function: :z=(_)
* Set the z component of this vector. Needs to be Num
*/
"foreign z=(z)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// vec4 ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Vec4
* _*Vector4*_
*/
"foreign class Vec4 {\n"
/*
* Constructor: :new()
* _*Constructor*_ Constructs a Vec4 intialized to X: 0.0f, Y: 0.0f, Z: 0.0f and W: 0.0f.
*/
"construct new() {}\n"
/*
* Constructor: :new(_)
* _*Constructor*_ Constructs a Vec4 intialized to X: v, Y: v, Z: v and W: v.
*
* Parameters:
* v - The value that both X, Y, Z and W will be initialized to. Needs to be a Num.
*/
"construct new(v) {}\n"
/*
* Constructor: :new(_,_)
* _*Constructor*_ Constructs a Vec4 intialized to X: x, Y: y, Z: z and W: w.
*
* Parameters:
* x - The value that X will be initialized to. Needs to be Num.
* y - The value that Y will be initialized to. Needs to be Num.
* z - The value that Z will be initialized to. Needs to be Num.
* w - The value that W will be initialized to. Needs to be Num.
*/
"construct new(x, y, z, w) {}\n"
/*
* Function: :normalize()
* Normalizes the current vector.
*/
"foreign normalize()\n"
/*
* Function: :dot(_)
* Get the dot product between the current vector and a provided vector.
*
* Parameters:
* v - The vector which the dot product will be done with
*/
"foreign dot(v)\n"
/*
* Function: :+(_)
* Add a value to the current value and return the result.
*
* Parameters:
* v - The value that will be added to this. Needs to be Num or Vec4
*/
"+(v) {\n"
"if (v is Num)  return Vec4.new(x + v, y + v, z + v, w + v)\n"
"if (v is Vec4) return Vec4.new(x + v.x, y + v.y, z + v.z, w + v.w)\n"
"}\n"
/*
* Function: :-(_)
* Subtract a value from the current value and return the result.
*
* Parameters:
* v - The value that will be subtracted from this. Needs to be Num or Vec4
*/
"-(v) {\n"
"if (v is Num)  return Vec4.new(x - v, y - v, z - v, w - v)\n"
"if (v is Vec4) return Vec4.new(x - v.x, y - v.y, z - v.z, w - v.w)\n"
"}\n"
/*
* Function: :*(_)
* Multiply a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be multiplied with. Needs to be Num or Vec4
*/
"*(v) {\n"
"if (v is Num)  return Vec4.new(x * v, y * v, z * v, w * v)\n"
"if (v is Vec4) return Vec4.new(x * v.x, y * v.y, z * v.z, w * v.w)\n"
"}\n"
/*
* Function: :/(_)
* Divide a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be divided with. Needs to be Num or Vec4
*/
"/(v) {\n"
"if (v is Num)  return Vec4.new(x / v, y / v, z / v, w / v)\n"
"if (v is Vec4) return Vec4.new(x / v.x, y / v.y, z / v.z, w / v.w)\n"
"}\n"
/*
* Function: :==(_)
* Check if this is equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec4
*/
"==(v) {\n"
"if (v is Num) return x == v && y == v && z == v && w == v\n"
"if (v is Vec4) return x == v.x && y == v.y && z == v.z && w == v.w\n"
"}\n"
/*
* Function: :!=(_)
* Check if this is not equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Vec4
*/
"!=(v) {\n"
"if (v is Num) return x != v || y != v || z != v || w != v\n"
"if (v is Vec4) return x != v.x || y != v.y || z != v.z || w != v.w\n"
"}\n"
/*
* Function: :normalized
* Get a normalized version of the current vector.
*/
"foreign normalized\n"
/*
* Function: :toString
* Get a stringified version of the current vector.
*/
"foreign toString\n"
/*
* Function: :x
* Get the x component of this vector.
*/
"foreign x\n"
/*
* Function: :y
* Get the y component of this vector.
*/
"foreign y\n"
/*
* Function: :z
* Get the z component of this vector.
*/
"foreign z\n"
/*
* Function: :w
* Get the w component of this vector.
*/
"foreign w\n"
/*
* Function: :magnitude
* Get the magnitude of this vector.
*/
"foreign magnitude\n"
/*
* Function: :magnitudeSqr
* Get the squared magnitude of this vector.
*/
"foreign magnitudeSqr\n"
/*
* Function: :length
* Get the length of this vector.
*/
"foreign length\n"
/*
* Function: :lengthSqr
* Get the squared length of this vector.
*/
"foreign lengthSqr\n"
/*
* Function: :x=(_)
* Set the x component of this vector. Needs to be Num
*/
"foreign x=(x)\n"
/*
* Function: :y=(_)
* Set the y component of this vector. Needs to be Num
*/
"foreign y=(y)\n"
/*
* Function: :z=(_)
* Set the z component of this vector. Needs to be Num
*/
"foreign z=(z)\n"
/*
* Function: :w=(_)
* Set the w component of this vector. Needs to be Num
*/
"foreign w=(w)\n"
"}\n"
/*
* Class: Quat
* _*Quaternion*_
*/
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// quat ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Quat
* _*Quaternion*_
*/
"foreign class Quat {\n"
/*
* Constructor: :new()
* _*Constructor*_ Constructs a Quat intialized to X: 0.0f, Y: 0.0f, Z: 0.0f and W: 1.0f.
*/
"construct new() {}\n"
/*
* Constructor: :new()
* _*Constructor*_ _Euler_ Constructs a Quat intialized using X, Y and Z in euler angles.
*
* Parameters:
* x - Euler angle used to initialize quaternion. Needs to be Num.
* y - Euler angle used to initialize quaternion. Needs to be Num.
* z - Euler angle used to initialize quaternion. Needs to be Num.
*/
"construct new(x, y, z) {}\n"
/*
* Constructor: :new(_,_)
* _*Constructor*_ _Quaternion_ Constructs a Quat intialized to X: x, Y: y, Z: z and W: w.
*
* Parameters:
* x - The value that X will be initialized to. Needs to be Num.
* y - The value that Y will be initialized to. Needs to be Num.
* z - The value that Z will be initialized to. Needs to be Num.
* w - The value that W will be initialized to. Needs to be Num.
*/
"construct new(x, y, z, w) {/*Quat*/}\n"
/*
* Function: :normalize()
* Normalizes the current quaternion.
*/
"foreign normalize()\n"
/*
* Function: :normalized
* Get a normalized version of the current quaternion.
*/
"foreign normalized\n"
/*
* Function: :toString
* Get a stringified version of the current quaternion.
*/
"foreign toString\n"
/*
* Function: :toEuler
* Gets the euler angles of the current quaternion.
*/
"foreign toEuler\n"
/*
* Function: :inverse()
* Inverts the current quaternion.
*/
"foreign inverse()\n"
/*
* Function: :inverted
* Returns an inverted version of the current quaternion.
*/
"foreign inverted\n"
/*
* Function: :*(_)
* Multiply a value with the current value and return the result.
*
* Parameters:
* v - The value that this will be multiplied with. Needs to be Quat or Vec3
*/
"*(v) {\n"
"if (v is Quat) return mulVec3(v)\n"
"if (v is Vec3) return mulQuat(v)\n"
"}\n"
"foreign mulVec3(vec3)\n"
"foreign mulQuat(quat)\n"
/*
* Function: :==(_)
* Check if this is equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Quat
*/
"	==(v) {\n"
"		if (v is Num) return x == v && y == v && z == v && w == v\n"
"		if (v is Quat) return x == v.x && y == v.y && z == v.z && w == v.w\n"
"	}\n"
/*
* Function: :!=(_)
* Check if this is not equal to another value.
*
* Parameters:
* v - The value that this will be checked against. Needs to be Num or Quat
*/
"!=(v) {\n"
"if (v is Num) return x != v || y != v || z != v || w != v\n"
"if (v is Quat) return x != v.x || y != v.y || z != v.z || w != v.w\n"
"}\n"
/*
* Function: :x
* Get the x component of this quaternion.
*/
"foreign x\n"
/*
* Function: :y
* Get the y component of this quaternion.
*/
"foreign y\n"
/*
* Function: :z
* Get the z component of this quaternion.
*/
"foreign z\n"
/*
* Function: :w
* Get the w component of this quaternion.
*/
"foreign w\n"
/*
* Function: :magnitude
* Get the magnitude of this quaternion.
*/
"foreign magnitude\n"
/*
* Function: :magnitudeSqr
* Get the squared magnitude of this quaternion.
*/
"foreign magnitudeSqr\n"
/*
* Function: :length
* Get the length of this quaternion.
*/
"foreign length\n"
/*
* Function: :lengthSqr
* Get the squared length of this quaternion.
*/
"foreign lengthSqr\n"
/*
* Function: :x=(_)
* Set the x component of this quaternion. Needs to be Num
*/
"foreign x=(x)\n"
/*
* Function: :y=(_)
* Set the y component of this quaternion. Needs to be Num
*/
"foreign y=(y)\n"
/*
* Function: :z=(_)
* Set the z component of this quaternion. Needs to be Num
*/
"foreign z=(z)\n"
/*
* Function: :w=(_)
* Set the w component of this quaternion. Needs to be Num
*/
"foreign w=(w)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// fast array //////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: FastArray
* _*Fast Array*_
*/
"foreign class FastArray {\n"
/*
* Constructor: :new(_)
* _*Constructor*_ Constructs a FastArray of the specified type.
*/
"construct new(type) {}\n"
/*
* Function: :at(_)
* Get the element at the specified index from this FastArray.
*
* Parameters:
* index - The index of the element that you want. Warning. Does not follow wrens negative number rules. Needs to be Num
*/
"foreign at(index)\n"
/*
* Function: :[_]
* Get the element at the specified index from this FastArray.
*
* Parameters:
* index - The index of the element that you want. Warning. Does not follow wrens negative number rules. Needs to be Num
*/
"[index] { at(index) }\n"
/*
* Function: :removeAt(_)
* Removes the element from this FastArray at the specified index.
*
* Parameters:
* index - The index of the element that you want to erase. Warning. Does not follow wrens negative number rules. Needs to be Num
*/
"foreign removeAt(index)\n"
/*
* Function: :isEmpty
* Get if the FastArray is empty.
*/
"foreign isEmpty\n"
/*
* Function: :count
* Get the number of elements in this FastArray.
*/
"foreign count\n"
/*
* Function: :type
* Get the type of the FastArray.
*/
"foreign type\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// texture /////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Texture
* _*Texture*_
*/
"foreign class Texture {\n"
/*
* Function: :load(_)
* _*Static*_ Loads a texture from a file using the file system.
*
* Parameters
* name - The name of the file that needs to be loaded. Needs to be String
*/
"foreign static load(name)\n"
/*
* Function: :loadCubeMap(_,_,_,_,_,_)
* _*Static*_ Loads six different textures and makes a cube map from it using the file system.
*
* Parameters
* front - The name of the file that needs to be loaded for the front. Needs to be String
* back - The name of the file that needs to be loaded for the back. Needs to be String
* top - The name of the file that needs to be loaded for the top. Needs to be String
* bottom - The name of the file that needs to be loaded for the bottom. Needs to be String
* left - The name of the file that needs to be loaded for the left. Needs to be String
* right - The name of the file that needs to be loaded for the right. Needs to be String
*/
"foreign static loadCubeMap(front, back, top, bottom, left, right)\n"
/*
* Function: :create(_,_)
* _*Static*_ Loads a texture from a file using the file system.
*
* Parameters
* size - The size of the texture. Needs to be Vec2
* format - The format of the texture. Needs to be TextureFormat
*/
"foreign static create(size, format)\n"
/*
* Function: :create(_,_,_)
* _*Static*_ Loads a texture from a file using the file system.
* _warning_: Bytes will be handled as an array of uint32's.
*
* Parameters
* size - The size of the texture. Needs to be Vec2
* bytes - The bytes that the texture will be initialized to. Needs to be uint32[]
* format - The format of the texture. Needs to be TextureFormat
*/
"foreign static create(size, bytes, format)\n"
/*
* Function: :size
* Get the size of the texture
*/
"foreign size\n"
/*
* Function: :format
* Get the format of the texture
*/
"foreign format\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// texture format //////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: TextureFormat
* Enum _*Texture Format*_
*/
"class TextureFormat {\n"
/*
* Function: :unknown
* Get the texture format identifying unknown
*/
"static unknown      { 0  }\n"
/*
* Function: :R8G8B8A8
* Get the texture format identifying R8G8B8A8
*/
"static R8G8B8A8     { 1  }\n"
/*
* Function: :R16G16B16A16
* Get the texture format identifying R16G16B16A16
*/
"static R16G16B16A16 { 2  }\n"
/*
* Function: :R32G32B32A32
* Get the texture format identifying R32G32B32A32
*/
"static R32G32B32A32 { 3  }\n"
/*
* Function: :R16G16
* Get the texture format identifying R16G16
*/
"static R16G16       { 4  }\n"
/*
* Function: :R32G32
* Get the texture format identifying R32G32
*/
"static R32G32       { 5  }\n"
/*
* Function: :R32
* Get the texture format identifying R32
*/
"static R32          { 6  }\n"
/*
* Function: :R16
* Get the texture format identifying R16
*/
"static R16          { 7  }\n"
/*
* Function: :R24G8
* Get the texture format identifying R24G8
*/
"static R24G8        { 8  }\n"
/*
* Function: :BC1
* Get the texture format identifying BC1
*/
"static BC1          { 9  }\n"
/*
* Function: :BC2
* Get the texture format identifying BC2
*/
"static BC2          { 10 }\n"
/*
* Function: :BC3
* Get the texture format identifying BC3
*/
"static BC3          { 11 }\n"
/*
* Function: :BC4
* Get the texture format identifying BC4
*/
"static BC4          { 12 }\n"
/*
* Function: :BC5
* Get the texture format identifying BC5
*/
"static BC5          { 13 }\n"
/*
* Function: :BC6
* Get the texture format identifying BC6
*/
"static BC6          { 14 }\n"
/*
* Function: :BC7
* Get the texture format identifying BC7
*/
"static BC7          { 15 }\n"
/*
* Function: :B8G8R8A8
* Get the texture format identifying B8G8R8A8
*/
"static B8G8R8A8     { 16 }\n"
/*
* Function: :A8
* Get the texture format identifying A8
*/
"static A8           { 17 }\n"
/*
* Function: :D32
* Get the texture format identifying D32
*/
"static D32          { 18 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// shader //////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Shader
* _*Shader*_
*/
"foreign class Shader {\n"
/*
* Function: :load(_)
* _*Static*_ Loads a shader from a file using the file system.
*
* Parameters
* name - The name of the file that needs to be loaded. Needs to be String
*/
"foreign static load(name)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// wave ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Wave
* _*Wave*_
*/
"foreign class Wave {\n"
/*
* Function: :load(_)
* _*Static*_ Loads a wave from a file using the file system.
*
* Parameters
* name - The name of the file that needs to be loaded. Needs to be String
*/
"foreign static load(name)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// mesh ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Mesh
* _*Mesh*_
*/
"foreign class Mesh {\n"
/*
* Function: :load(_)
* _*Static*_ Loads a mesh from a file using the file system.
*
* Parameters
* name - The name of the file that needs to be loaded. Needs to be String
*/
"foreign static load(name)\n"
/*
* Function: :generate(_)
* _*Static*_ Generates a mesh based on the type specified.
*
* Allowed types:
* - cube
* - cylinder
* - sphere
*
* Parameters
* name - The type of object that needs to be generated. Needs to be String
*/
"foreign static generate(type)\n"
/*
* Function: :generateCube(_,_)
* _*Static*_ Generates a cube mesh based on the min and max specified.
*
* Parameters
* min - The min of the cube generated. Needs to be Vec3
* max - The max of the cube generated. Needs to be Vec3
*/
"foreign static generateCube(min, max)\n"
/*
* Function: :create()
* _*Static*_ Creates an empty mesh.
*/
"foreign static create()\n"
/*
* Function: :subMeshCount
* Get the number of submeshes contained in this mesh.
*/
"foreign subMeshCount\n"
/*
* Function: :positions
* Get the positions contained in this mesh.
*/
"foreign positions\n"
/*
* Function: :positions=(_)
* Set the positions contained in this mesh.
*/
"foreign positions=(vec)\n"
/*
* Function: :normals
* Get the normals contained in this mesh.
* 
* Parameters
* vec - The new positions. Needs to be Vec3[]
*/
"foreign normals\n"
/*
* Function: :normals=(_)
* Set the normals contained in this mesh.
*/
"foreign normals=(vec)\n"
/*
* Function: :texCoords
* Get the texture coordinates contained in this mesh.
* 
* Parameters
* vec - The new normals. Needs to be Vec3[]
*/
"foreign texCoords\n"
/*
* Function: :texCoords=(_)
* Set the texture coordinates contained in this mesh.
* 
* Parameters
* vec - The new texture coordinates. Needs to be Vec2[]
*/
"foreign texCoords=(vec)\n"
/*
* Function: :colours
* Get the colours contained in this mesh.
*/
"foreign colours\n"
/*
* Function: :colours=(_)
* Set the colours contained in this mesh.
* 
* Parameters
* vec - The new colours. Needs to be Vec4[]
*/
"foreign colours=(vec)\n"
/*
* Function: :tangents
* Get the tangents contained in this mesh.
*/
"foreign tangents\n"
/*
* Function: :tangents=(_)
* Set the tangents contained in this mesh.
* 
* Parameters
* vec - The new tangents. Needs to be Vec3[]
*/
"foreign tangents=(vec)\n"
/*
* Function: :joints
* Get the joints contained in this mesh.
*/
"foreign joints\n"
/*
* Function: :joints=(_)
* Set the joints contained in this mesh.
* 
* Parameters
* vec - The new joints. Needs to be Vec4[]
*/
"foreign joints=(vec)\n"
/*
* Function: :weights
* Get the weights contained in this mesh.
*/
"foreign weights\n"
/*
* Function: :weights=(_)
* Set the weights contained in this mesh.
* 
* Parameters
* vec - The new weights. Needs to be Vec4[]
*/
"foreign weights=(vec)\n"
/*
* Function: :indices
* Get the indices contained in this mesh.
*/
"foreign indices\n"
/*
* Function: :indices=(_)
* Set the indices contained in this mesh.
* 
* Parameters
* vec - The new indices. Needs to be Num[]
*/
"foreign indices=(vec)\n"
/*
* Function: :recalculateTangents()
* Recalculates the tangents based on the positions, normals and indices of the mesh.
*/
"foreign recalculateTangents()\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// gameObject //////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: GameObject
* _*Game Object*_
*/
"foreign class GameObject {\n"

/*
* Function: :new()
* Creates a new game object.
*/
"construct new() {\n"
"addComponent(Transform)\n"
"}\n"
/*
* Function: :destroy()
* Destroys this game object and all of its components.
*/
"foreign destroy()\n"
/*
* Function: :addComponent(_)
* Adds a component of the specified type and returns a component of that type.
* 
* Parameters
* component - The type of the component that you want to add. Needs to be a component.
*/
"    addComponent(component) {\n"
"      var c = component.new()\n"
"      c.goAdd(this)\n"
"      return c\n"
"    }\n"
/*
* Function: :getComponent(_)
* Gets a component of the specified type.
* 
* Parameters
* component - The type of the component that you want to get. Needs to be a component.
*/
"getComponent(component) { component.goGet(this) }\n"
/*
* Function: :removeComponent(_)
* Removes a component of the specified type.
* 
* Parameters
* component - The type of the component that you want to remove. Needs to be a component.
*/
"removeComponent(component) {\n"
"getComponent(component).goRemove(this)\n"
"}\n"
/*
* Function: :==(_)
* Check if this is equal to another game object.
* 
* Parameters
* v - The game object that you want to compare it to. Needs to be a GameObject.
*/
"==(v) { id == v.id }\n"
/*
* Function: :==(_)
* Check if this is not equal to another game object.
* 
* Parameters
* v - The game object that you want to compare it to. Needs to be a GameObject.
*/
"!=(v) { id != v.id }\n"
/*
* Function: :[_]
* Gets a component of the specified type.
* 
* Parameters
* component - The type of the component that you want to get. Needs to be a component.
*/
"[component] { getComponent(component) }\n"
/*
* Function: :name
* Get the name of the game object.
*/
"foreign name\n"
/*
* Function: :name=(_)
* Set the name of the game object.
* 
* Parameters
* name - The new name of the game object. Needs to be String.
*/
"foreign name=(name)\n"
/*
* Function: :tags
* Get the tags of the game object.
*/
"foreign tags\n"
/*
* Function: :tags=(_)
* Set the tags of the game object.
* 
* Parameters
* name - The new name of the game object. Needs to be String[].
*/
"foreign tags=(tags)\n"
/*
* Function: :transform
* Get the transform of the game object.
*/
"transform { getComponent(Transform) }\n"
/*
* Function: :toString
* Stringifies the ID of the game object.
*/
"toString { \"[%(id)]\" }\n"
/*
* Function: :id
* Get the ID of this game object.
*/
"foreign id\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// Transform ///////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Transform
* _*Transform*_
*/
"foreign class Transform {\n"
"	construct new() {}\n"
"	toString { \"[%(gameObject.id): Transform]\" }\n"
"\n"
"	foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is Transform) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is Transform) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"	foreign goAdd(gameObject)\n"
"	foreign static goGet(gameObject)\n"
"	foreign goRemove(gameObject)\n"
"\n"
"	moveLocal(movement) { localPosition = localPosition + movement }\n"
"	moveWorld(movement) { worldPosition = worldPosition + movement }\n"
"	scaleLocal(scale) { localScale = localScale * scale }\n"
"	scaleWorld(scale) { worldScale = worldScale * scale }\n"
"	rotateEulerLocal(euler) { localEuler = localEuler + euler }\n"
"	rotateEulerWorld(euler) { worldEuler = worldEuler + euler }\n"
"\n"
"	foreign parent\n"
"	foreign parent=(parent)\n"
"	foreign children\n"
"\n"
"	foreign worldPosition\n"
"	foreign worldPosition=(p)\n"
"	foreign worldScale\n"
"	foreign worldScale=(s)\n"
"	foreign worldRotation\n"
"	foreign worldRotation=(e)\n"
"	foreign worldEuler\n"
"	foreign worldEuler=(e)\n"
"	foreign worldForward\n"
"	foreign worldRight\n"
"	foreign worldUp\n"
"\n"
"	foreign localPosition\n"
"	foreign localPosition=(p)\n"
"	foreign localScale\n"
"	foreign localScale=(s)\n"
"	foreign localRotation\n"
"	foreign localRotation=(e)\n"
"	foreign localEuler\n"
"	foreign localEuler=(e)\n"
"	foreign localForward\n"
"	foreign localRight\n"
"	foreign localUp\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// camera projection ///////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: CameraProjection
* _*Camera Projection*_
*/
"foreign class CameraProjection {\n"
"  static Perspective { 0 }\n"
"  static Ortho { 1 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// camera //////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Camera
* _*Camera*_
*/
"foreign class Camera {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): Camera]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is Camera) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is Camera) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign addShaderPass(name, shader, input, output)\n"
"	foreign ndcToWorld(ndc)\n"
"\n"
"    foreign near\n"
"    foreign near=(near)\n"
"    foreign far\n"
"    foreign far=(far)\n"
"    foreign fovDeg\n"
"    foreign fovDeg=(deg)\n"
"    foreign fovRad\n"
"    foreign fovRad=(rad)\n"
"    foreign width\n"
"    foreign width=(width)\n"
"    foreign height\n"
"    foreign height=(height)\n"
"    foreign projection\n"
"    foreign projection=(projection)\n"
"    fov { fovRad }\n"
"    fov(rad) { fovRad = rad }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// msehRender //////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: MeshRender
* _*Mesh Render*_
*/
"foreign class MeshRender {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): MeshRender]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is MeshRender) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is MeshRender) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign attach(mesh)\n"
"\n"
"    foreign makeStatic()\n"
"    foreign makeStaticRecursive()\n"
"    foreign makeDynamic()\n"
"    foreign makeDynamicRecursive()\n"
"\n"
"		foreign metallicnessFactor\n"
"		foreign metallicnessFactor=(metallicnessFactor)\n"
"		foreign roughnessFactor\n"
"		foreign roughnessFactor=(roughnessFactor)\n"
"		foreign emissivenessFactor\n"
"		foreign emissivenessFactor=(emissivenessFactor)\n"
"\n"
"    foreign mesh\n"
"    foreign mesh=(mesh)\n"
"    foreign subMesh\n"
"    foreign subMesh=(subMesh)\n"
"    foreign albedo\n"
"    foreign albedo=(albedo)\n"
"    foreign normal\n"
"    foreign normal=(normal)\n"
"    foreign DMRA\n"
"    foreign DMRA=(dmra)\n"
"    foreign emissive\n"
"    foreign emissive=(emissive)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// lod /////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Lod
* _*Level Of Detail*_
*/
"foreign class Lod {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): Lod]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is Lod) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is Lod) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign addLod(mesh, distance)\n"
"    foreign addLodRecursive(mesh, distance)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// rigidBody ///////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: RigidBody
* _*Rigid Body*_
*/
"class RigidBody {\n"
"	 construct new() {\n"
"  }\n"
"  toString { \"[%(gameObject.id): RigidBody]\" }\n"
"\n"
"  gameObject { _go }\n"
"\n"
"	 ==(v) {\n"
"		 if (v is RigidBody) {\n"
"			 return gameObject == v.gameObject\n"
"		 } else {\n"
"			 return false\n"
"		 }\n"
"	 }\n"
"	 !=(v) {\n"
"		 if (v is RigidBody) {\n"
"			 return gameObject != v.gameObject\n"
"		 } else {\n"
"			 return true\n"
"		 }\n"
"	 }\n"
"\n"
"	 goAdd(gameObject) {\n"
"		 goAddForeign(gameObject)\n"
"    goSet(gameObject)\n"
"  }\n"
"\n"
"  goSet(gameObject) {\n"
"    _go = gameObject\n"
"	 }\n"
"\n"
"  static goGet(gameObject) {\n"
"		 var v = RigidBody.new()\n"
"		 v.goSet(gameObject)\n"
"	   return v\n"
"	 }\n"
"\n"
"	 foreign goAddForeign(gameObject)\n"
"\n"
"  goRemove(gameObject) {}\n"
"\n"
"  foreign priv_applyImpulse(go, impulse)\n"
"  foreign priv_velocity(go)\n"
"  foreign priv_velocity(go, velocity)\n"
"  foreign priv_angularVelocity(go)\n"
"  foreign priv_angularVelocity(go, angularVelocity)\n"
"  foreign priv_velocityConstraints(go)\n"
"  foreign priv_velocityConstraints(go, velocityConstraints)\n"
"  foreign priv_angularConstraints(go)\n"
"  foreign priv_angularConstraints(go, angularConstraints)\n"
"  foreign priv_friction(go)\n"
"  foreign priv_friction(go, friction)\n"
"  foreign priv_mass(go)\n"
"  foreign priv_mass(go, mass)\n"
"\n"
"  applyImpulse(impulse)                     { priv_applyImpulse(_go, impulse) }\n"
"  velocity                                  { priv_velocity(_go) }\n"
"  velocity=(velocity)                       { priv_velocity(_go, velocity) }\n"
"  angularVelocity                           { priv_angularVelocity(_go) }\n"
"  angularVelocity=(angularVelocity)         { priv_angularVelocity(_go, angularVelocity) }\n"
"  velocityConstraints                       { priv_velocityConstraints(_go) }\n"
"  velocityConstraints=(velocityConstraints) { priv_velocityConstraints(_go, velocityConstraints) }\n"
"  angularConstraints                        { priv_angularConstraints(_go) }\n"
"  angularConstraints=(angularConstraints)   { priv_angularConstraints(_go, angularConstraints) }\n"
"	 friction                                  { priv_friction(_go) }\n"
"	 friction=(friction)                       { priv_friction(_go, friction) }\n"
"	 mass                                      { priv_mass(_go) }\n"
"	 mass=(mass)                               { priv_mass(_go, mass) }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// waveSource //////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: WaveSource
* _*Wave Source*_
*/
"foreign class WaveSource {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): WaveSource]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is WaveSource) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is WaveSource) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign play()\n"
"    foreign pause()\n"
"    foreign stop()\n"
"    foreign makeMainListener()\n"
"\n"
"    foreign buffer\n"
"    foreign buffer=(buffer)\n"
"    foreign state\n"
"    foreign relativeToListener\n"
"    foreign relativeToListener=(relative)\n"
"    foreign loop\n"
"    foreign loop=(loop)\n"
"    foreign offset=(offset)\n"
"    foreign volume\n"
"    foreign volume=(volume)\n"
"    foreign gain\n"
"    foreign gain=(gain)\n"
"    foreign pitch\n"
"    foreign pitch=(pitch)\n"
"    foreign radius\n"
"    foreign radius=(radius)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// collider ////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Collider
* _*Collider*_
*/
"foreign class Collider {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): Collider]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is Collider) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is Collider) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign makeBoxCollider()\n"
"    makeCubeCollider() { makeBoxCollider() }\n"
"    foreign makeSphereCollider()\n"
"    foreign makeCapsuleCollider()\n"
"    foreign makeMeshCollider(mesh, subMesh)\n"
"    foreign makeMeshColliderRecursive(mesh)\n"
"\n"
"    foreign layers\n"
"    foreign layers=(layers)\n"
"    foreign layersRecursive=(layers)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// light ///////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Light
* _*Light*_
*/
"foreign class Light {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): Light]\" }\n"
"\n"
"    foreign gameObject\n"
"\n"
"	==(v) {\n"
"		if (v is Light) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is Light) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    foreign goAdd(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemove(gameObject)\n"
"\n"
"    foreign type\n"
"    foreign type=(type)\n"
"    foreign isRSM\n"
"    foreign isRSM=(isRSM)\n"
"    foreign lightColour\n"
"    foreign lightColour=(colour)\n"
"    foreign ambientColour\n"
"    foreign ambientColour=(colour)\n"
"    foreign lightIntensity\n"
"    foreign lightIntensity=(intensity)\n"
"    foreign shadowType\n"
"    foreign shadowType=(shadowType)\n"
"    foreign shadowMapSizePx\n"
"    foreign shadowMapSizePx=(shadowMapSizePx)\n"
"    foreign depth\n"
"    foreign depth=(depth)\n"
"    foreign size\n"
"    foreign size=(size)\n"
"    foreign enabled\n"
"    foreign enabled=(enabled)\n"
"    foreign texture\n"
"    foreign texture=(texture)\n"
"    foreign innerCutOff\n"
"    foreign innerCutOff=(innerCutOff)\n"
"    foreign outerCutOff\n"
"    foreign outerCutOff=(outerCutOff)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// light types /////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: LightTypes
* Enum _*Light Types*_
*/
"class LightTypes {\n"
/*
* Function: :Unknown
* _*Static*_ Get the light type Unknown.
*/
"static Unknown { 0 }\n"
/*
* Function: :Directional
* _*Static*_ Get the light type Directional.
*/
"static Directional { 1 }\n"
/*
* Function: :Spot
* _*Static*_ Get the light type Spot.
*/
"static Spot { 2 }\n"
/*
* Function: :Point
* _*Static*_ Get the light type Point.
*/
"static Point { 3 }\n"
/*
* Function: :Cascade
* _*Static*_ Get the light type Cascade.
*/
"static Cascade { 4 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// shadow types ////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: ShadowTypes
* Enum _*Shadow Types*_
*/
"class ShadowTypes {\n"
/*
* Function: :Off
* _*Static*_ Get the shadow type Off.
*/
"static Off { 0 }\n"
/*
* Function: :Generated
* _*Static*_ Get the shadow type Generated.
*/
"static Generated { 1 }\n"
/*
* Function: :GenerateOnce
* _*Static*_ Get the shadow type GenerateOnce.
*/
"static GenerateOnce { 2 }\n"
/*
* Function: :Dynamic
* _*Static*_ Get the shadow type Dynamic.
*/
"static Dynamic { 3 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// mono_behaviour //////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: MonoBehaviour
* _*Mono Behaviour*_
*/
"class MonoBehaviour {\n"
"    construct new() {\n"
"    }\n"
"    toString { \"[%(gameObject.id): MonoBehaviour]\" }\n"
"\n"
"    gameObject { _game_object }\n"
"    transform { _transform }\n"
"\n"
"	==(v) {\n"
"		if (v is MonoBehaviour) {\n"
"			return gameObject == v.gameObject\n"
"		} else {\n"
"			return false\n"
"		}\n"
"	}\n"
"	!=(v) {\n"
"		if (v is MonoBehaviour) {\n"
"			return gameObject != v.gameObject\n"
"		} else {\n"
"			return true\n"
"		}\n"
"	}\n"
"\n"
"    goAdd(gameObject) {\n"
"        _game_object = gameObject\n"
"        _transform = gameObject.transform\n"
"        goAddPrivate(gameObject)\n"
"        initialize()\n"
"    }\n"
"\n"
"    goRemove(gameObject) {\n"
"        deinitialize()\n"
"        goRemovePrivate(gameObject)\n"
"    }\n"
"\n"
"    initialize() {}\n"
"    deinitialize() {}\n"
"\n"
"    update() {}\n"
"    fixedUpdate() {}\n"
"\n"
"    onCollisionEnter(other, normal) {}\n"
"    onCollisionStay(other, normal)  {}\n"
"    onCollisionExit(other, normal)  {}\n"
"\n"
"    onTriggerEnter(other, normal) {}\n"
"    onTriggerStay(other, normal)  {}\n"
"    onTriggerExit(other, normal)  {}\n"
"\n"
"    foreign goAddPrivate(gameObject)\n"
"    foreign static goGet(gameObject)\n"
"    foreign goRemovePrivate(gameObject)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// graphics ////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Graphics
* _*Graphics*_
*/
"class Graphics {\n"
"    foreign static vsync=(enabled)\n"
"    foreign static vsync\n"
"    foreign static renderScale=(scale)\n"
"    foreign static renderScale\n"
"    foreign static setLightShaders(generate, modify, modifyCount, publish, shadowType)\n"
"    foreign static windowSize\n"
"    foreign static aspectRatio\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// gui /////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: GUI
* _*Gaphical User Interface*_
*/
"class GUI {\n"
"    foreign static enabled=(enabled)\n"
"    foreign static enabled\n"
"    foreign static loadURL(url)\n"
"    foreign static executeJavaScript(string)\n"
"    foreign static bindCallback(name, object)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// postProcess /////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: PostProcess
* _*Post Process*_
*/
"class PostProcess {\n"
"    foreign static addRenderTarget(name, render_scale, format)\n"
"    foreign static addRenderTarget(name, texture)\n"
"    foreign static setRenderTargetFlag(name, flag, value)\n"
"    foreign static setFinalRenderTarget(name)\n"
"    foreign static addShaderPass(name, shader, input, output)\n"
"    foreign static setShaderPassEnabled(name, enabled)\n"
"    foreign static irradianceConvolution(input, output)\n"
"    foreign static hammerhead(input, output)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// render target flags /////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: RenderTargetFlags
* Enum _*Render Target Flags*_
*/
"class RenderTargetFlags {\n"
/*
* Function: :DynamicScale
* _*Static*_ Dynamic Scale flag.
*/
"static DynamicScale { 1 << 1 }\n"
/*
* Function: :DynamicData
* _*Static*_ Dynamic Data flag.
*/
"static DynamicData { 1 << 2 }\n"
/*
* Function: :Clear
* _*Static*_ Clear flag.
*/
"static Clear { 1 << 3 }\n"
/*
* Function: :Recreate
* _*Static*_ Recreate flag.
*/
"static Recreate { 1 << 4 }\n"
/*
* Function: :IsRenderTarget
* _*Static*_ Is Render Target flag.
*/
"static IsRenderTarget { 1 << 5 }\n"
/*
* Function: :FromDDS
* _*Static*_ From DDS flag.
*/
"static FromDDS { 1 << 6 }\n"
/*
* Function: :ContainsAlpha
* _*Static*_ Contains Alpha flag.
*/
"static ContainsAlpha { 1 << 7 }\n"
/*
* Function: :Resize
* _*Static*_ Resize flag.
*/
"static Resize { 1 << 8 }\n"
"}"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// input ///////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Input
* _*Input*_
*/
"class Input {\n"
"    foreign static getKey(key)\n"
"    foreign static getAxis(axis)\n"
"    foreign static getButton(button)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// axes ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Axes
* Enum _*Axes*_
*/
"class Axes {\n"
/*
* Function: :LeftStickX
* _*Static*_ Get the input axis LeftStickX.
*/
"static LeftStickX { 0  }\n"
/*
* Function: :LeftStickY
* _*Static*_ Get the input axis LeftStickY.
*/
"static LeftStickY { 1  }\n"
/*
* Function: :RightStickX
* _*Static*_ Get the input axis RightStickX.
*/
"static RightStickX { 2  }\n"
/*
* Function: :RightStickY
* _*Static*_ Get the input axis RightStickY.
*/
"static RightStickY { 3  }\n"
/*
* Function: :LeftTrigger
* _*Static*_ Get the input axis LeftTrigger.
*/
"static LeftTrigger { 4  }\n"
/*
* Function: :RightTrigger
* _*Static*_ Get the input axis RightTrigger.
*/
"static RightTrigger { 5  }\n"
/*
* Function: :MouseX
* _*Static*_ Get the input axis MouseX.
*/
"static MouseX { 20 }\n"
/*
* Function: :MouseY
* _*Static*_ Get the input axis MouseY.
*/
"static MouseY { 21 }\n"
/*
* Function: :Scroll
* _*Static*_ Get the input axis Scroll.
*/
"static Scroll { 22 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// buttons /////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Buttons
* Enum _*Buttons*_
*/
"class Buttons {\n"
/*
* Function: :A
* _*Static*_ Get the input button A.
*/
"static A { 0  }\n"
/*
* Function: :B
* _*Static*_ Get the input button B.
*/
"static B { 1  }\n"
/*
* Function: :X
* _*Static*_ Get the input button X.
*/
"static X { 2  }\n"
/*
* Function: :Y
* _*Static*_ Get the input button Y.
*/
"static Y { 3  }\n"
/*
* Function: :Up
* _*Static*_ Get the input button Up.
*/
"static Up { 4  }\n"
/*
* Function: :Down
* _*Static*_ Get the input button Down.
*/
"static Down { 5  }\n"
/*
* Function: :Left
* _*Static*_ Get the input button Left.
*/
"static Left { 6  }\n"
/*
* Function: :Right
* _*Static*_ Get the input button Right.
*/
"static Right { 7  }\n"
/*
* Function: :LeftBumper
* _*Static*_ Get the input button LeftBumper.
*/
"static LeftBumper { 8  }\n"
/*
* Function: :RightBumper
* _*Static*_ Get the input button RightBumper.
*/
"static RightBumper { 9  }\n"
/*
* Function: :LeftStick
* _*Static*_ Get the input button LeftStick.
*/
"static LeftStick { 0  }\n"
/*
* Function: :RightStick
* _*Static*_ Get the input button RightStick.
*/
"static RightStick { 11 }\n"
/*
* Function: :Start
* _*Static*_ Get the input button Start.
*/
"static Start { 12 }\n"
/*
* Function: :Back
* _*Static*_ Get the input button Back.
*/
"static Back { 13 }\n"
/*
* Function: :LMB
* _*Static*_ Get the input button LMB.
*/
"static LMB { 20 }\n"
/*
* Function: :LeftMouseButton
* _*Static*_ Get the input button LeftMouseButton.
*/
"static LeftMouseButton { 20 }\n"
/*
* Function: :MMB
* _*Static*_ Get the input button MMB.
*/
"static MMB { 21 }\n"
/*
* Function: :MiddleMouseButton
* _*Static*_ Get the input button MiddleMouseButton.
*/
"static MiddleMouseButton { 21 }\n"
/*
* Function: :RMB
* _*Static*_ Get the input button RMB.
*/
"static RMB { 22 }\n"
/*
* Function: :RightMouseButton
* _*Static*_ Get the input button RightMouseButton.
*/
"static RightMouseButton { 22 }\n"
/*
* Function: :LMBD
* _*Static*_ Get the input button LMBD.
*/
"static LMBD { 23 }\n"
/*
* Function: :MMBD
* _*Static*_ Get the input button MMBD.
*/
"static MMBD { 24 }\n"
/*
* Function: :RMBD
* _*Static*_ Get the input button RMBD.
*/
"static RMBD { 25 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// keys ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Keys
* Enum _*Keys*_
*/
"class Keys {\n"
/*
* Function: :A
* _*Static*_ Get the input key A.
*/
"static A { 65   }\n"
/*
* Function: :B
* _*Static*_ Get the input key B.
*/
"static B { 66   }\n"
/*
* Function: :C
* _*Static*_ Get the input key C.
*/
"static C { 67   }\n"
/*
* Function: :D
* _*Static*_ Get the input key D.
*/
"static D { 68   }\n"
/*
* Function: :E
* _*Static*_ Get the input key E.
*/
"static E { 69   }\n"
/*
* Function: :F
* _*Static*_ Get the input key F.
*/
"static F { 70   }\n"
/*
* Function: :G
* _*Static*_ Get the input key G.
*/
"static G { 71   }\n"
/*
* Function: :H
* _*Static*_ Get the input key H.
*/
"static H { 72   }\n"
/*
* Function: :I
* _*Static*_ Get the input key I.
*/
"static I { 73   }\n"
/*
* Function: :J
* _*Static*_ Get the input key J.
*/
"static J { 74   }\n"
/*
* Function: :K
* _*Static*_ Get the input key K.
*/
"static K { 75   }\n"
/*
* Function: :L
* _*Static*_ Get the input key L.
*/
"static L { 76   }\n"
/*
* Function: :M
* _*Static*_ Get the input key M.
*/
"static M { 77   }\n"
/*
* Function: :N
* _*Static*_ Get the input key N.
*/
"static N { 78   }\n"
/*
* Function: :O
* _*Static*_ Get the input key O.
*/
"static O { 79   }\n"
/*
* Function: :P
* _*Static*_ Get the input key P.
*/
"static P { 80   }\n"
/*
* Function: :Q
* _*Static*_ Get the input key Q.
*/
"static Q { 81   }\n"
/*
* Function: :R
* _*Static*_ Get the input key R.
*/
"static R { 82   }\n"
/*
* Function: :S
* _*Static*_ Get the input key S.
*/
"static S { 83   }\n"
/*
* Function: :T
* _*Static*_ Get the input key T.
*/
"static T { 84   }\n"
/*
* Function: :U
* _*Static*_ Get the input key U.
*/
"static U { 85   }\n"
/*
* Function: :V
* _*Static*_ Get the input key V.
*/
"static V { 86   }\n"
/*
* Function: :W
* _*Static*_ Get the input key W.
*/
"static W { 87   }\n"
/*
* Function: :X
* _*Static*_ Get the input key X.
*/
"static X { 88   }\n"
/*
* Function: :Y
* _*Static*_ Get the input key Y.
*/
"static Y { 89   }\n"
/*
* Function: :Z
* _*Static*_ Get the input key Z.
*/
"static Z { 90   }\n"
/*
* Function: :Tilde
* _*Static*_ Get the input key Tilde.
*/
"static Tilde { 126  }\n"
/*
* Function: :Num0
* _*Static*_ Get the input key Num0.
*/
"static Num0 { 48   }\n"
/*
* Function: :Num1
* _*Static*_ Get the input key Num1.
*/
"static Num1 { 49   }\n"
/*
* Function: :Num2
* _*Static*_ Get the input key Num2.
*/
"static Num2 { 50   }\n"
/*
* Function: :Num3
* _*Static*_ Get the input key Num3.
*/
"static Num3 { 51   }\n"
/*
* Function: :Num4
* _*Static*_ Get the input key Num4.
*/
"static Num4 { 52   }\n"
/*
* Function: :Num5
* _*Static*_ Get the input key Num5.
*/
"static Num5 { 53   }\n"
/*
* Function: :Num6
* _*Static*_ Get the input key Num6.
*/
"static Num6 { 54   }\n"
/*
* Function: :Num7
* _*Static*_ Get the input key Num7.
*/
"static Num7 { 55   }\n"
/*
* Function: :Num8
* _*Static*_ Get the input key Num8.
*/
"static Num8 { 56   }\n"
/*
* Function: :Num9
* _*Static*_ Get the input key Num9.
*/
"static Num9 { 57   }\n"
/*
* Function: :F1
* _*Static*_ Get the input key F1.
*/
"static F1 { 0x70 }\n"
/*
* Function: :F2
* _*Static*_ Get the input key F2.
*/
"static F2 { 0x71 }\n"
/*
* Function: :F3
* _*Static*_ Get the input key F3.
*/
"static F3 { 0x72 }\n"
/*
* Function: :F4
* _*Static*_ Get the input key F4.
*/
"static F4 { 0x73 }\n"
/*
* Function: :F5
* _*Static*_ Get the input key F5.
*/
"static F5 { 0x74 }\n"
/*
* Function: :F6
* _*Static*_ Get the input key F6.
*/
"static F6 { 0x75 }\n"
/*
* Function: :F7
* _*Static*_ Get the input key F7.
*/
"static F7 { 0x76 }\n"
/*
* Function: :F8
* _*Static*_ Get the input key F8.
*/
"static F8 { 0x77 }\n"
/*
* Function: :F9
* _*Static*_ Get the input key F9.
*/
"static F9 { 0x78 }\n"
/*
* Function: :F10
* _*Static*_ Get the input key F10.
*/
"static F10 { 0x79 }\n"
/*
* Function: :F11
* _*Static*_ Get the input key F11.
*/
"static F11 { 0x7A }\n"
/*
* Function: :F12
* _*Static*_ Get the input key F12.
*/
"static F12 { 0x7B }\n"
/*
* Function: :F13
* _*Static*_ Get the input key F13.
*/
"static F13 { 0x7C }\n"
/*
* Function: :F14
* _*Static*_ Get the input key F14.
*/
"static F14 { 0x7D }\n"
/*
* Function: :F15
* _*Static*_ Get the input key F15.
*/
"static F15 { 0x7E }\n"
/*
* Function: :F16
* _*Static*_ Get the input key F16.
*/
"static F16 { 0x7F }\n"
/*
* Function: :F17
* _*Static*_ Get the input key F17.
*/
"static F17 { 0x80 }\n"
/*
* Function: :F18
* _*Static*_ Get the input key F18.
*/
"static F18 { 0x81 }\n"
/*
* Function: :F19
* _*Static*_ Get the input key F19.
*/
"static F19 { 0x82 }\n"
/*
* Function: :F20
* _*Static*_ Get the input key F20.
*/
"static F20 { 0x83 }\n"
/*
* Function: :F21
* _*Static*_ Get the input key F21.
*/
"static F21 { 0x84 }\n"
/*
* Function: :F22
* _*Static*_ Get the input key F22.
*/
"static F22 { 0x85 }\n"
/*
* Function: :F23
* _*Static*_ Get the input key F23.
*/
"static F23 { 0x86 }\n"
/*
* Function: :F24
* _*Static*_ Get the input key F24.
*/
"static F24 { 0x87 }\n"
/*
* Function: :F25
* _*Static*_ Get the input key F25.
*/
"static F25 { 0x88 }\n"
/*
* Function: :Numpad0
* _*Static*_ Get the input key Numpad0.
*/
"static Numpad0 { 0x60 }\n"
/*
* Function: :Numpad1
* _*Static*_ Get the input key Numpad1.
*/
"static Numpad1 { 0x61 }\n"
/*
* Function: :Numpad2
* _*Static*_ Get the input key Numpad2.
*/
"static Numpad2 { 0x62 }\n"
/*
* Function: :Numpad3
* _*Static*_ Get the input key Numpad3.
*/
"static Numpad3 { 0x63 }\n"
/*
* Function: :Numpad4
* _*Static*_ Get the input key Numpad4.
*/
"static Numpad4 { 0x64 }\n"
/*
* Function: :Numpad5
* _*Static*_ Get the input key Numpad5.
*/
"static Numpad5 { 0x65 }\n"
/*
* Function: :Numpad6
* _*Static*_ Get the input key Numpad6.
*/
"static Numpad6 { 0x66 }\n"
/*
* Function: :Numpad7
* _*Static*_ Get the input key Numpad7.
*/
"static Numpad7 { 0x67 }\n"
/*
* Function: :Numpad8
* _*Static*_ Get the input key Numpad8.
*/
"static Numpad8 { 0x68 }\n"
/*
* Function: :Numpad9
* _*Static*_ Get the input key Numpad9.
*/
"static Numpad9 { 0x69 }\n"
/*
* Function: :Tab
* _*Static*_ Get the input key Tab.
*/
"static Tab { 0x09 }\n"
/*
* Function: :Backspace
* _*Static*_ Get the input key Backspace.
*/
"static Backspace { 0x08 }\n"
/*
* Function: :Insert
* _*Static*_ Get the input key Insert.
*/
"static Insert { 0x2D }\n"
/*
* Function: :Delete
* _*Static*_ Get the input key Delete.
*/
"static Delete { 0x2E }\n"
/*
* Function: :Return
* _*Static*_ Get the input key Return.
*/
"static Return { 0x0D }\n"
/*
* Function: :Enter
* _*Static*_ Get the input key Enter.
*/
"static Enter { 0x0D }\n"
/*
* Function: :Space
* _*Static*_ Get the input key Space.
*/
"static Space { 0x20 }\n"
/*
* Function: :Minus
* _*Static*_ Get the input key Minus.
*/
"static Minus { 0xBD }\n"
/*
* Function: :Shift
* _*Static*_ Get the input key Shift.
*/
"static Shift { 0x10 }\n"
/*
* Function: :LShift
* _*Static*_ Get the input key LShift.
*/
"static LShift { 0xA0 }\n"
/*
* Function: :RShift
* _*Static*_ Get the input key RShift.
*/
"static RShift { 0xA1 }\n"
/*
* Function: :Control
* _*Static*_ Get the input key Control.
*/
"static Control { 0x11 }\n"
/*
* Function: :LControl
* _*Static*_ Get the input key LControl.
*/
"static LControl { 0xA2 }\n"
/*
* Function: :RControl
* _*Static*_ Get the input key RControl.
*/
"static RControl { 0xA3 }\n"
/*
* Function: :Left
* _*Static*_ Get the input key Left.
*/
"static Left { 0x25 }\n"
/*
* Function: :UP
* _*Static*_ Get the input key Up.
*/
"static Up { 0x26 }\n"
/*
* Function: :Right
* _*Static*_ Get the input key Right.
*/
"static Right { 0x27 }\n"
/*
* Function: :Down
* _*Static*_ Get the input key Down.
*/
"static Down { 0x28 }\n"
/*
* Function: :PageUp
* _*Static*_ Get the input key PageUp.
*/
"static PageUp { 0x21 }\n"
/*
* Function: :PageDown
* _*Static*_ Get the input key PageDown.
*/
"static PageDown { 0x22 }\n"
/*
* Function: :Home
* _*Static*_ Get the input key Home.
*/
"static Home { 0x24 }\n"
/*
* Function: :End
* _*Static*_ Get the input key End.
*/
"static End { 0x23 }\n"
/*
* Function: :CapsLock
* _*Static*_ Get the input key CapsLock.
*/
"static CapsLock { 0x14 }\n"
/*
* Function: :ScrollLock
* _*Static*_ Get the input key ScrollLock.
*/
"static ScrollLock { 0x91 }\n"
/*
* Function: :NumLock
* _*Static*_ Get the input key NumLock.
*/
"static NumLock { 0x90 }\n"
/*
* Function: :PrintScreen
* _*Static*_ Get the input key PrintScreen.
*/
"static PrintScreen { 0x2C }\n"
/*
* Function: :Pause
* _*Static*_ Get the input key Pause.
*/
"static Pause { 0x13 }\n"
/*
* Function: :Decimal
* _*Static*_ Get the input key Decimal.
*/
"static Decimal { 0x6E }\n"
/*
* Function: :Divide
* _*Static*_ Get the input key Divide.
*/
"static Divide { 0x6F }\n"
/*
* Function: :Multiply
* _*Static*_ Get the input key Multiply.
*/
"static Multiply { 0x6A }\n"
/*
* Function: :Subtract
* _*Static*_ Get the input key Subtract.
*/
"static Subtract { 0x6D }\n"
/*
* Function: :Add
* _*Static*_ Get the input key Add.
*/
"static Add { 0x6B }\n"
/*
* Function: :Equal
* _*Static*_ Get the input key Equal.
*/
"static Equal { 0xBB }\n"
/*
* Function: :LeftBracket
* _*Static*_ Get the input key LeftBracket.
*/
"static LeftBracket { 219  }\n"
/*
* Function: :RightBracket
* _*Static*_ Get the input key RightBracket.
*/
"static RightBracket { 221  }\n"
/*
* Function: :BackSlash
* _*Static*_ Get the input key BackSlash.
*/
"static BackSlash { 0xE2 }\n"
/*
* Function: :Semicolon
* _*Static*_ Get the input key Semicolon.
*/
"static Semicolon { 186  }\n"
/*
* Function: :Apostrophe
* _*Static*_ Get the input key Apostrophe.
*/
"static Apostrophe { 0xDE }\n"
/*
* Function: :Grave
* _*Static*_ Get the input key Grave.
*/
"static Grave { 126  }\n"
/*
* Function: :Comma
* _*Static*_ Get the input key Comma.
*/
"static Comma { 0xBC }\n"
/*
* Function: :Period
* _*Static*_ Get the input key Period.
*/
"static Period { 0xBE }\n"
/*
* Function: :Slash
* _*Static*_ Get the input key Slash.
*/
"static Slash { 191  }\n"
/*
* Function: :LAlt
* _*Static*_ Get the input key LAlt.
*/
"static LAlt { 0x12 }\n"
/*
* Function: :LSuper
* _*Static*_ Get the input key LSuper.
*/
"static LSuper { 0x5B }\n"
/*
* Function: :RAlt
* _*Static*_ Get the input key RAlt.
*/
"static RAlt { 0x12 }\n"
/*
* Function: :RSuper
* _*Static*_ Get the input key RSuper.
*/
"static RSuper { 0x5C }\n"
/*
* Function: :Menu
* _*Static*_ Get the input key Menu.
*/
"static Menu { 0xA4 }\n"
/*
* Function: :Escape
* _*Static*_ Get the input key Escape.
*/
"static Escape { 0x1B }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// math ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Math
* _*Math*_
*/
"class Math {\n"
"    foreign static random(min, max)\n"
"    foreign static random(max)\n"
"    foreign static random()\n"
"    foreign static seed=(seed)\n"
"    foreign static hash(string)\n"
"\n"
"    foreign static clamp(val, min, max)\n"
"    foreign static clamp(val)\n"
"\n"
"    foreign static wrap(val, min, max)\n"
"    foreign static wrapMax(val, max)\n"
"\n"
"    foreign static lerp(lhs, rhs, val)\n"
"    foreign static lerp2(lhs, rhs, val)\n"
"    foreign static lerp3(lhs, rhs, val)\n"
"    foreign static lerp4(lhs, rhs, val)\n"
"    foreign static lerpq(lhs, rhs, val)\n"
"    foreign static slerp(lhs, rhs, val)\n"
"\n"
"    foreign static sqrt(val)\n"
"    foreign static sqr(val)\n"
"    foreign static pow(val, pow)\n"
"\n"
"    foreign static min(lhs, rhs)\n"
"    foreign static max(lhs, rhs)\n"
"\n"
"    foreign static abs(val)\n"
"    foreign static floor(val)\n"
"    foreign static ceil(val)\n"
"    foreign static round(val)\n"
"\n"
"    foreign static cos(val)\n"
"    foreign static cosh(val)\n"
"    foreign static acos(val)\n"
"\n"
"    foreign static sin(val)\n"
"    foreign static sinh(val)\n"
"    foreign static asin(val)\n"
"\n"
"    foreign static tan(val)\n"
"    foreign static tanh(val)\n"
"    foreign static atan(val)\n"
"\n"
"    foreign static atan2(y, x)\n"
"\n"
"    foreign static sign(x)\n"
"\n"
"    foreign static lookRotation(forward, up)\n"
"\n"
"    static pi { 3.14159265359 }\n"
"    static tau { 6.28318530718 }\n"
"    static deg2Rad { 0.0174533 }\n"
"    static degToRad { 0.0174533 }\n"
"    static rad2Deg { 57.2958 }\n"
"    static radToDeg { 57.2958 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// time ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Time
* _*Time*_
*/
"class Time {\n"
"    foreign static fixedDeltaTime\n"
"    foreign static deltaTime\n"
"	foreign static timeScale=(timeScale)\n"
"	foreign static timeScale\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// physics constraints /////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: PhysicsConstraints
* Enum _*Physics Constraints*_
*/
"class PhysicsConstraints {\n"
/*
* Function: :None
* _*Static*_ Get the physic constraint type None.
*/
"static None { 0 }\n"
/*
* Function: :X
* _*Static*_ Get the physic constraint type X.
*/
"static X { 1 << 0 }\n"
/*
* Function: :Y
* _*Static*_ Get the physic constraint type Y.
*/
"static Y { 1 << 1 }\n"
/*
* Function: :Z
* _*Static*_ Get the physic constraint type Z.
*/
"static Z { 1 << 2 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// sort ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Sort
* _*Sort*_
*/
"class Sort {\n"
"  static swap(arr, l, r) {\n"
"    var t = arr[l]\n"
"    arr[l] = arr[r]\n"
"    arr[r] = t\n"
"    return arr\n"
"  }\n"
"\n"
"  static bubbleSort(arr, sorter) {\n"
"    var i = 0\n"
"    while (i < arr.count - 1) {\n"
"      // Last i elements are already in place\n"
"      var j = 0\n"
"      while (j < arr.count - i - 1) {\n"
"        if (sorter.greater(arr[j], arr[j + 1])) {\n"
"          arr = swap(arr, j, j + 1)\n"
"        }\n"
"\n"
"        j = j + 1\n"
"      }\n"
"\n"
"      i = i + 1\n"
"    }\n"
"\n"
"    return arr\n"
"  }\n"
"\n"
"  // Default to bubble sort.\n"
"  static sort(arr, sorter) { bubbleSort(arr, sorter) }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// debug ///////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Debug
* _*Debug*_
*/
"class Debug {\n"
"    foreign static drawLine(from, to, color)\n"
"    foreign static drawTri(p1, p2, p3, color)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// manifold ////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Manifold
* _*Manifold*_
*/
"foreign class Manifold {\n"
"	foreign gameObject\n"
"	foreign normal\n"
"	foreign depth\n"
"	foreign point\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// physics /////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Physics
* _*Physics*_
*/
"class Physics {\n"
"    foreign static gravity\n"
"    foreign static gravity=(gravity)\n"
"    foreign static castRay(from, to)\n"
"\n"
"	foreign static debugDrawEnabled\n"
"	foreign static debugDrawEnabled=(debugDrawEnabled)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// file ////////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: File
* _*File*_
*/
"foreign class File {\n"
"    construct open(file) {}\n"
"    construct open(file, writeMode) {}\n"
"    foreign close()\n"
"    foreign read()\n"
"    foreign write(text)\n"
"\n"
"    static readOnly  { 1 }\n"
"    static writeOnly { 2 }\n"
"    static readWrite { 3 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// noise ///////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Noise
* _*Noise*_
*/
"foreign class Noise {\n"
"    construct new() {}\n"
"\n"
"    foreign seed\n"
"    foreign seed=(seed)\n"
"    foreign frequency\n"
"    foreign frequency=(frequency)\n"
"    foreign interpolation\n"
"    foreign interpolation=(interpolation)\n"
"\n"
"    foreign getPerlin(id)\n"
"    foreign getPerlinFractal(id)\n"
"    foreign getCellular(id)\n"
"    foreign getCubic(id)\n"
"    foreign getCubicFractal(id)\n"
"    foreign getSimplex(id)\n"
"    foreign getSimplexFractal(id)\n"
"    foreign getValue(id)\n"
"    foreign getValueFractal(id)\n"
"    foreign getWhiteNoise(id)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// noise interpolation /////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: NoiseInterpolation
* Enum _*Noise Interpolation*_
*/
"foreign class NoiseInterpolation {\n"
/*
* Function: :Linear
* _*Static*_ Get the noise interpolation type Linear.
*/
"static Linear { 0 }\n"
/*
* Function: :Hermite
* _*Static*_ Get the noise interpolation type Hermite.
*/
"static Hermite { 1 }\n"
/*
* Function: :Quintic
* _*Static*_ Get the noise interpolation type Quintic.
*/
"static Quintic { 2 }\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// nav mesh node ///////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: NavMeshNode
* _*Navigation Mesh Node*_
*/
"foreign class NavMeshNode {\n"
/*
* Function: :addConnection(_)
* Adds a connection between this node and the specified node.
*
* Parameters:
* node - The node that you want this node to be connected to.
*/
"foreign addConnection(node)\n"
/*
* Function: :removeConnection(_)
* Removes the connection between this node and the specified node.
*
* Parameters:
* node - The node that you want the connection to be broken with.
*/
"foreign removeConnection(node)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// nav mesh ////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: NavMesh
* _*Navigation Mesh*_
*/
"foreign class NavMesh {\n"
/*
* Constructor: :new()
* Constructs an empty navigation mesh.
*/
"construct new() {}\n"
/*
* Function: :addNode(_)
* Adds a node to this navigation mesh at the specified position.
* Returns the added node.
*
* Parameters:
* position - The position that you want this node to be at.
*/
"foreign addNode(position)\n"
/*
* Function: :removeNode(_)
* Removes a node from this navigation mesh at the specified position.
*
* Parameters:
* node - The node that you want to remove.
*/
"foreign removeNode(node)\n"
/*
* Function: :findPath(_,_)
* Finds the best path over the navigation mesh between two positions.
*
* Parameters:
* from - The position that you want to search from.
* to - The position that you want to search to.
*/
"foreign findPath(from, to)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// nav mesh promise ////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: NavMeshPromise
* _*Navigation Mesh Promise*_
*/
"foreign class NavMeshPromise {\n"
/*
* Function: :finished
* Is the promise finished?
*/
"foreign finished\n"
/*
* Function: :path
* The found path.
*/
"foreign path\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// tri nav mesh ////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: TriNavMesh
* _*Triangle Navigation Mesh*_
*/
"foreign class TriNavMesh {\n"
/*
* Constructor: :new()
* Constructs an empty navigation mesh.
*/
"construct new() {}\n"
/*
* Function: :addTri(_,_,_)
* Adds a triangle to the navigation mesh.
* Needs to be in clockwise order.
*
* Parameters:
* a - Point a of the triangle. Needs to be Vec3.
* b - Point b of the triangle. Needs to be Vec3.
* c - Point c of the triangle. Needs to be Vec3.
*/
"foreign addTri(a, b, c)\n"
/*
* Function: :addTriHole(_,_,_)
* Adds a triangluar hole to the navigation mesh.
* Needs to be in clockwise order.
*
* Parameters:
* a - Point a of the triangle hole. Needs to be Vec3.
* b - Point b of the triangle hole. Needs to be Vec3.
* c - Point c of the triangle hole. Needs to be Vec3.
*/
"foreign addTriHole(a, b, c)\n"
/*
* Function: :addQuad(_,_,_)
* Adds a quad to the navigation mesh using an AABB.
*
* Parameters:
* min - Min of the AABB quad. Needs to be Vec3.
* max - Max of the AABB quad. Needs to be Vec3.
*/
"foreign addQuad(min, max)\n"
/*
* Function: :addQuadHole(_,_,_)
* Adds a quad hole to the navigation mesh using an AABB.
*
* Parameters:
* min - Min of the AABB quad hole. Needs to be Vec3.
* max - Max of the AABB quad hole. Needs to be Vec3.
*/
"foreign addQuadHole(min, max)\n"
/*
* Function: :generate()
* generates the navigation mesh.
*/
"foreign getTriangles()\n"
/*
* Function: :findPath(_,_)
* Finds the best path over the navigation mesh between two positions.
*
* Parameters:
* from - The position that you want to search from.
* to - The position that you want to search to.
*/
"foreign findPath(from, to)\n"
/*
* Function: :findPathPromise(_,_)
* Finds the best path over the navigation mesh between two positions.
* Returns a promise which will be filled at a later moment.
*
* Parameters:
* from - The position that you want to search from.
* to - The position that you want to search to.
*/
"foreign findPathPromise(from, to)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// assert //////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: Assert
* _*Assert*_
*/
"class Assert {\n"
"    foreign static isTrue(expr)\n"
"    foreign static isTrue(expr, msg)\n"
"    foreign static isFalse(expr)\n"
"    foreign static isFalse(expr, msg)\n"
"    foreign static throw(msg)\n"
"}\n"

"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
"///// world ///////////////////////////////////////////////////////////////////////////////////////\n"
"///////////////////////////////////////////////////////////////////////////////////////////////////\n"
/*
* Class: World
* _*World*_
*/
"class World {\n"
"    foreign static serialize(file)\n"
"    foreign static deserialize(file)\n"
"}\n"
;
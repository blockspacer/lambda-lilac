# Console
* **static debug(string)** Outputs a string using the *debug* channel. Adds a *new line* at the end of the string.
* **static error(string)** Outputs a string using the *error* channel. Adds a *new line* at the end of the string.
* **static info(string)** Outputs a string using the *info* channel. Adds a *new line* at the end of the string.
* **static warning(string)** Outputs a string using the *warning* channel. Adds a *new line* at the end of the string.

# Vec2
* **new()** Constructs a *Vec2* with *x* and *y* set to *0*.
* **new(value)** Constructs a *Vec2* with *x* and *y* set to *value*.
* **new(x, y)** Constructs a *Vec2* with *x* and *y* set to the input *x* and *y*.
* **normalize()** Normalizes the vector. 
* **dot(other)** Takes the dot product of *this* *Vec2* and the *other* *Vec2*.
* **+(val)** Can be used to add this *Vec2* to another *Vec2* or a *number*.
* **-(val)** Can be used to subtract this *Vec2* to another *Vec2* or a *number*.
* ***(val)** Can be used to multiply this *Vec2* to another *Vec2* or a *number*.
* **/(val)** Can be used to divide this *Vec2* to another *Vec2* or a *number*.
* **normalized** Returns a normalized variant of this *Vec2*.
* **toString** Returns a *string* version of this *Vec2* in the format *[x, y]*.
* **x** Returns the *x* component of this *Vec2*.
* **y** Returns the *y* component of this *Vec2*.
* **magnitude** Returns the *length* / *magnitude* of this *Vec2*.
* **magnitudeSqr** Returns the *length squared* / *magnitude squared* of this *Vec2*.
* **length** Returns the *length* / *magnitude* of this *Vec2*.
* **lengthSqr** Returns the *length squared* / *magnitude squared* of this *Vec2*.
* **x=(value)** Sets the *x* component of this *Vec2* to *value*.
* **y=(value)** Sets the *y* component of this *Vec2* to *value*.

# Vec3
* **new()** Constructs a *Vec3* with *x* , *y* and *z* set to *0*.
* **new(value)** Constructs a *Vec3* with *x*, *y* and *z* set to *value*.
* **new(x, y, z)** Constructs a *Vec3* with *x*, *y* and *z* set to the input *x*, *y* and *z*.
* **normalize()** Normalizes the vector. 
* **dot(other)** Takes the dot product of *this* *Vec3* and the *other* *Vec3*.
* **+(val)** Can be used to add this *Vec3* to another *Vec3* or a *number*.
* **-(val)** Can be used to subtract this *Vec3* to another *Vec3* or a *number*.
* ***(val)** Can be used to multiply this *Vec3* to another *Vec3* or a *number*.
* **/(val)** Can be used to divide this *Vec3* to another *Vec3* or a *number*.
* **normalized** Returns a normalized variant of this *Vec3*.
* **toString** Returns a *string* version of this *Vec3* in the format *[x, y, z]*.
* **x** Returns the *x* component of this *Vec3*.
* **y** Returns the *y* component of this *Vec3*.
* **z** Returns the *z* component of this *Vec3*.
* **magnitude** Returns the *length* / *magnitude* of this *Vec3*.
* **magnitudeSqr** Returns the *length squared* / *magnitude squared* of this *Vec3*.
* **length** Returns the *length* / *magnitude* of this *Vec3*.
* **lengthSqr** Returns the *length squared* / *magnitude squared* of this *Vec3*.
* **x=(value)** Sets the *x* component of this *Vec3* to *value*.
* **y=(value)** Sets the *y* component of this *Vec3* to *value*.
* **z=(value)** Sets the *z* component of this *Vec3* to *value*.

# Vec4
* **new()** Constructs a *Vec4* with *x* , *y*, *z* and *w* set to *0*.
* **new(value)** Constructs a *Vec4* with *x*, *y*, *z* and *w* set to *value*.
* **new(x, y, z, w)** Constructs a *Vec4* with *x*, *y*, *z* and *w* set to the input *x*, *y*, *z* and *w*.
* **normalize()** Normalizes the vector. 
* **dot(other)** Takes the dot product of *this* *Vec4* and the *other* *Vec4*.
* **+(val)** Can be used to add this *Vec4* to another *Vec4* or a *number*.
* **-(val)** Can be used to subtract this *Vec4* to another *Vec4* or a *number*.
* ***(val)** Can be used to multiply this *Vec4* to another *Vec4* or a *number*.
* **/(val)** Can be used to divide this *Vec4* to another *Vec4* or a *number*.
* **normalized** Returns a normalized variant of this *Vec4*.
* **toString** Returns a *string* version of this *Vec4* in the format *[x, y, z, w]*.
* **x** Returns the *x* component of this *Vec4*.
* **y** Returns the *y* component of this *Vec4*.
* **z** Returns the *z* component of this *Vec4*.
* **w** Returns the *w* component of this *Vec4*.
* **magnitude** Returns the *length* / *magnitude* of this *Vec4*.
* **magnitudeSqr** Returns the *length squared* / *magnitude squared* of this *Vec4*.
* **length** Returns the *length* / *magnitude* of this *Vec4*.
* **lengthSqr** Returns the *length squared* / *magnitude squared* of this *Vec4*.
* **x=(value)** Sets the *x* component of this *Vec4* to *value*.
* **y=(value)** Sets the *y* component of this *Vec4* to *value*.
* **z=(value)** Sets the *z* component of this *Vec4* to *value*.
* **w=(value)** Sets the *w* component of this *Vec4* to *value*.

# Quat
* **new()** Constructs a *Quat* with *x* , *y* and *z* set to *0* and *w* set to *1*.
* **new(x, y, z)** Constructs a *Quat* where *x*, *y* and *z* are *euler angles*.
* **new(x, y, z, w)** Constructs a *Quat* with *x*, *y*, *z* and *w* set to the input *x*, *y*, *z* and *w*.
* **normalize()** Normalizes the vector. 
* **normalized** Returns a normalized variant of this *Quat*.
* **toString** Returns a *string* version of this *Quat* in the format *[x, y, z, w]*.
* **x** Returns the *x* component of this *Quat*.
* **y** Returns the *y* component of this *Quat*.
* **z** Returns the *z* component of this *Quat*.
* **w** Returns the *w* component of this *Quat*.
* **magnitude** Returns the *length* / *magnitude* of this *Quat*.
* **magnitudeSqr** Returns the *length squared* / *magnitude squared* of this *Quat*.
* **length** Returns the *length* / *magnitude* of this *Quat*.
* **lengthSqr** Returns the *length squared* / *magnitude squared* of this *Quat*.
* **x=(value)** Sets the *x* component of this *Quat* to *value*.
* **y=(value)** Sets the *y* component of this *Quat* to *value*.
* **z=(value)** Sets the *z* component of this *Quat* to *value*.
* **w=(value)** Sets the *w* component of this *Quat* to *value*.

# Texture
* **static load(name)** Loads a texture from the texture cache with the provided name. Returns the loaded texture.
* **static loadCubeMap(front, back, top, bottom, left, right)** Creates a cube map from the 6 textures loaded from the texture cache with the provided names. Returns the loaded cube map.
* **static create(size, format)** Creates a texture with the given size and format. Useful for the creation of render targets since you do not require preset data for them.
* **static create(size, bytes, format)** Creates a texture with the given size, format and data.
* **size** Returns the size of the texture.
* **format** Returns the format of the texture.

# TextureFormat
* **static Unknown** Value of: 0
* **static R8G8B8A8** Value of: 1
* **static R16G16B16A16** Value of: 2
* **static R32G32B32A32** Value of: 3
* **static R16G16** Value of: 4
* **static R32G32** Value of: 5
* **static R32** Value of: 6
* **static R16** Value of: 7
* **static R24G8** Value of: 8
* **static BC1** Value of: 9
* **static BC2** Value of: 10
* **static BC3** Value of: 11
* **static BC4** Value of: 12
* **static BC5** Value of: 13
* **static BC6** Value of: 14
* **static BC7** Value of: 15

# Shader
* **static load(name)** Loads a shader from the shader cache with the provided name. Returns the loaded shader.
* **setVariableFloat1(name, value)** Sets the variable of a given name to the provided value. Expects a float as input.
* **setVariableFloat2(name, value)** Sets the variable of a given name to the provided value. Expects a Vec2 as input.
* **setVariableFloat3(name, value)** Sets the variable of a given name to the provided value. Expects a Vec3 as input.
* **setVariableFloat4(name, value)** Sets the variable of a given name to the provided value. Expects a Vec4 as input.

# Wave
* **static load(name)** Loads a wave from the wave cache with the provided name. Returns the loaded wave.

# Mesh
* **static load(name)** Loads a mesh from the mesh cache with the provided name. Returns the loaded mesh.
* **static generate(type)** Generates a mesh of the provided type. The supported types are: *cube*, *cylinder* and *sphere*. Returns the generated mesh.
* **static create()** Creates an empty mesh. Returns the created mesh.
* TODO (Hilze): Continue with this.
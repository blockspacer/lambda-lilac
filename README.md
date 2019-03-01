# Project Lambda

<h2>About the project</h2>
Project lambda is a personal engine project that I am working on with a friend.<br>
We have been developing this engine for a few months and it is already rich in features.<br>
I started this side project to learn about all the different things that make up an engine. Thus far I have been focusing on graphics, but I'd like to know what else is required to make an engine work.<br>
Thus far I have learned how to implement multiple scripting languages (LUA, C# using Mono and AngelScript).<br>
I have also learned how to design the systems that make the engine function, like for instance the world and the entity component system.<br>
<br>
Some of the features that we have currently implemented are:<br>
<table>
  <tr><th><p>Graphics</th><td><b>DirectX11</b> and <b>Metal</b><sup>1</sup></p></td></tr>
  <tr><th><p>Windows</th><td><b>Win32</b>, <b>GLFW</b> and <b>SDL2</b><sup>2</sup></p></td></tr>
  <tr><th><p>GUI</th><td><b>UltraLight</b></p></td></tr>
  <tr><th><p>Scripting</th><td><b>Wren</b> and <b>AngelScript</b><sup>3</sup></p></td></tr>
  <tr><th><p>Audio</th><td><b>SoLoud</b></p></td></tr>
  <tr><th><p>Physics</th><td><b>Bullet3</b></p></td></tr>
  <tr><th><p>Networking</th><td><b>ENet</b><sup>3</sup></p></td></tr>
  <tr><th><p>Tools</th><td><b>ShaderConductor</b>, <b>DirectXTex</b>, <b>TinyGLTF</b><sup>1</sup> and <b>Assimp</b><sup>3</sup></p></td></tr>
</table>
<b>1)</b> In progress<br/>
<b>2)</b> Does not work on Windows<br/>
<b>3)</b> Deprecated<br/>
<br/>
I however have not skipped on the graphics department. These are the post processing effects that I have currently implemented:<br>
<li>Physically Based Rendering (PBR)</li>
<li>Image Based Lighting (IBL)</li>
<li>Tone Mapping</li>
<li>Bloom</li>
<li>Fast Approximate Anti-Aliasing (FXAA)</li>
<li>Shadow Mapping</li>

<h2>Documentation</h2>
You can check out the documentation for our scripting implementation <a href="docs/index.html">here</a>.

<h2>About the team</h2>
<div style="background-color:white; width: 750px; border-right:1px solid #616161;">
  <div style="background-color:lightgray">
    <h2 style="color:black; margin: 0px; padding: 5px;">Hilze Vonck</h2>
  </div>
  <div style="color:black;">
    <table style="margin: 0px; padding: 0px; border-collapse:collapse;">
      <tr>
        <td style="border:none; padding:0px;">
          <img src="docs/img/hilze.jpg" style="padding:0px; width:200px;"/>
        </td>
        <td style="border:none; padding:5px">
          <p>TODO (Hilze): Fill in.</p>
        </td>
      </tr>
    </table>
  </div>
  <div style="background-color:#616161; padding: 5px; margin: 0px; color:white">
    <p style="margin: 0px;">You can check out his portfolio <a href="http://www.hilzevonck.me">here</a></p>
  </div>
</div>

<div style="background-color:white; width: 750px; border-left:1px solid #616161;">
  <div style="background-color:lightgray">
    <h2 style="color:black; margin: 0px; padding: 5px;">Manolito van der Want</h2>
  </div>
  <div style="color:black">
    <table style="margin: 0px; padding: 0px; border-collapse:collapse;">
      <tr>
        <td style="border:none; padding:5px">
          <p>TODO (Mano): Fill in.</p>
        </td>
        <td style="border:none; padding:0px;">
          <img src="docs/img/mano.jpg" style="width:200px"/>
        </td>
      </tr>
    </table>
  </div>
  <div style="background-color:#616161; padding: 5px; margin: 0px; color:white">
    <p style="margin: 0px;">You can check out his portfolio <a href="https://soundcloud.com/manolito-van-der-want-1">here</a></p>
  </div>
</div>

# How to clone this repository
<table>
  <tr>
    <td>Clone this repo</td>
    <td>git clone https://github.com/hvonck/lambda-lilac.git</td>
  </tr>
  <tr>
    <td>Make sure that you have LFS installed</td>
    <td>git lfs install --skip-smudge</td>
  </tr>
    <td>Fetch all LFS files</td>
    <td>git lfs pull</td>
  </tr>
  <tr>
    <td>Initialize all submodules</td>
    <td>git submodule init</td>
  </tr>
  <tr>
    <td>Update all submodules</td>
    <td>git submodule update --recursive --remote</td>
  </tr>
  <tr>
    <td>If you want to build tools</td>
    <td>Install Python 2.7.x</td>
  </tr>
  <tr>
    <td>Run CMAKE</td>
    <td>cmake -S . -B ./bin/</td>
  </tr>
</table>

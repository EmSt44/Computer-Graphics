#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

uniform vec3 u_lightPosition; // The position of your light source

uniform bool u_shaderToggle;
// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
// ...

// Vertex shader outputs
out vec3 v_color;
out vec3 v_N; //view-space normal
out vec3 v_L; //view-space light vector
out vec3 v_V; //view vector
// ...

void main()
{

      //Model view projection matrix
      mat4 mvp = u_projection*u_view*u_model;

      gl_Position = mvp*a_position;


      mat4 mv = u_view*u_model;

      // Transform the vertex position to view space (eye coordinates)
      vec3 positionEye = vec3(mv * a_position);

      // Calculate the view-space normal
      v_N = normalize(mat3(mv) * a_normal);

      // Calculate the view-space light direction
      v_L = normalize(u_lightPosition - positionEye);

      // Viewer vector
      v_V = normalize(-positionEye);
      
      v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color
    }
    

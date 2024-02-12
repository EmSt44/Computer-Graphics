#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

uniform vec3 u_diffuseColor; // The diffuse surface color of the model
uniform vec3 u_lightPosition; // The position of your light source
uniform vec3 u_ambientColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;

uniform bool u_shaderToggle;
// ...

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
// ...

// Vertex shader outputs
out vec3 v_color;
out vec3 N; //view-space normal
out vec3 L; //view-space light vector
out vec3 V; //view vector
// ...

void main()
{
    if (u_shaderToggle) {
          //Model view projection matrix
    mat4 mvp = u_projection*u_view*u_model;

    gl_Position = mvp*a_position;


    mat4 mv = u_view*u_model;

    // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

    // Calculate the view-space normal
    vec3 N = normalize(mat3(mv) * a_normal);

    // Calculate the view-space light direction
    vec3 L = normalize(u_lightPosition - positionEye);

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));

    // Multiply the diffuse reflection term with the base surface color
    vec3 diffuse_illumination = diffuse * u_diffuseColor;

    // Viewer vector
    vec3 V = normalize(-positionEye);

    // Halfway vector
    vec3 H = normalize(L + V);

    float specular = pow(dot(N, H), u_specularPower); 

    vec3 specular_illumination = specular * u_specularColor;

    // Add ambient, diffuse and specular for Blinn-Phong shading
    v_color = u_ambientColor + diffuse_illumination + specular_illumination;
       
    }
    else {
        // Modify color
    v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color

       //Model view projection matrix
    mat4 mvp = u_projection*u_view*u_model;

    gl_Position = mvp*a_position;
        }
    }
    

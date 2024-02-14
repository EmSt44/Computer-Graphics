#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform vec3 u_diffuseColor; // The diffuse surface color of the model
uniform vec3 u_lightPosition; // The position of your light source
uniform vec3 u_ambientColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;

uniform bool u_shaderToggle;
uniform bool u_gammaToggle;
// ...

// Fragment shader inputs
in vec3 v_color;
in vec3 v_N;
in vec3 v_L;
in vec3 v_V;
// ...

// Fragment shader outputs
out vec4 frag_color;

void main()
{
    if(u_shaderToggle) {

        // Calculate the diffuse (Lambertian) reflection term
        float diffuse = max(0.0, dot(v_N, v_L));

        // Multiply the diffuse reflection term with the base surface color
        vec3 diffuse_illumination = diffuse * u_diffuseColor;

 
        // Halfway vector
        vec3 H = normalize(v_L + v_V);

        float specular = pow(dot(v_N, H), u_specularPower); 

        float specular_normalization_factor = (u_specularPower + 8)/8;

        vec3 specular_illumination = specular * u_specularColor * specular_normalization_factor;

        //Calculate the blinn phong shading model
        vec3 blinn_phong = u_ambientColor + diffuse_illumination + specular_illumination;

        if (u_gammaToggle) {
            blinn_phong = pow(blinn_phong, vec3(1 / 2.2));
        }

        frag_color = vec4(blinn_phong, 1.0);
    }
    else {
        frag_color = vec4(v_color, 1.0);
    }
    
}

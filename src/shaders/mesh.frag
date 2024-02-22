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
uniform bool u_cubemapToggle;

uniform samplerCube u_cubemap;
// ...

// Fragment shader inputs
in vec3 v_color;
in vec3 N;
in vec3 L;
in vec3 V;
// ...

// Fragment shader outputs
out vec4 frag_color;

void main()
{
    if(u_shaderToggle) {

        // Reflection vector
        vec3 R = reflect(-V, N);

        // Calculate the diffuse (Lambertian) reflection term
        float diffuse = max(0.0, dot(N, L));

        // Multiply the diffuse reflection term with the base surface color
        vec3 diffuse_illumination = diffuse * u_diffuseColor;

 
        // Halfway vector
        vec3 H = normalize(L + V);

        float specular = pow(dot(N, H), u_specularPower); 

        float specular_normalization_factor = (u_specularPower + 8)/8;

        vec3 specular_illumination = specular * u_specularColor * specular_normalization_factor;

        //Calculate the blinn phong shading model
        vec3 blinn_phong = u_ambientColor + diffuse_illumination + specular_illumination;

        if (u_cubemapToggle) {
            vec3 color = texture(u_cubemap, R).rgb;
            frag_color = vec4(color, 1.0);
        }
        else if (u_gammaToggle) {
            blinn_phong = pow(blinn_phong, vec3(1 / 2.2));
            frag_color = vec4(blinn_phong, 1.0);
        }
        else {
            frag_color = vec4(blinn_phong, 1.0);
        }

    }
    else {
        frag_color = vec4(v_color, 1.0);
    }
    
}

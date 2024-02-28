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

uniform bool u_texcoordToggle;

uniform sampler2D u_texture1;
uniform bool u_textureToggle;
uniform bool u_textureExists;

uniform sampler2D u_shadowmapTexture;
uniform mat4 u_shadowMatrix;
uniform bool u_shadowmapToggle;
uniform float u_bias;
// ...

// Fragment shader inputs
in vec3 v_color;
in vec3 N;
in vec3 L;
in vec3 V;
in vec2 v_texcoord_0;
in vec4 v_mp;
// ...

// Fragment shader outputs
out vec4 frag_color;

float shadowmap_visibility(sampler2D shadowmap, vec4 shadowPos, float bias)
{
    vec2 delta = vec2(0.5) / textureSize(shadowmap, 0).xy;
    vec2 texcoord = (shadowPos.xy / shadowPos.w) * 0.5 + 0.5;
    float depth = (shadowPos.z / shadowPos.w) * 0.5 + 0.5;
    
    // Sample the shadowmap and compare texels with (depth - bias) to
    // return a visibility value in range [0, 1]. If you take more
    // samples (using delta to offset the texture coordinate), the
    // returned value should be the average of all comparisons.
    float texel = texture(shadowmap, texcoord).r;
    float visibility = float(texel > depth - bias);
    return visibility;
}

void main()
{
    if(u_shaderToggle) {
        vec4 shadowPos = u_shadowMatrix * v_mp;
        float visibility = shadowmap_visibility(u_shadowmapTexture, shadowPos, u_bias);

        // Reflection vector
        vec3 R = reflect(-V, N);

       
        
        // Calculate the diffuse (Lambertian) reflection term
        float diffuse = max(0.0, dot(N, L));

        

        // Multiply the diffuse reflection term with the base surface color
        vec3 diffuse_illumination = diffuse * u_diffuseColor;
        if (u_shadowmapToggle) {
            diffuse_illumination = diffuse_illumination * visibility;
        }

 
        // Halfway vector
        vec3 H = normalize(L + V);

        float specular = pow(dot(N, H), u_specularPower); 

        float specular_normalization_factor = (u_specularPower + 8)/8;


        vec3 specular_illumination = specular * u_specularColor * specular_normalization_factor;
        if(u_shadowmapToggle) {
            specular_illumination = specular_illumination * visibility;
        }
       

        //Calculate the blinn phong shading model
        vec3 blinn_phong = u_ambientColor + diffuse_illumination + specular_illumination;

        if (u_cubemapToggle) {
            vec3 color = texture(u_cubemap, R).rgb;
            frag_color = vec4(color, 1.0);
        }
        else if (u_textureToggle && u_textureExists) {
            vec3 texture_color = texture(u_texture1, v_texcoord_0).rgb;
            if (u_shadowmapToggle) {
                texture_color = texture_color * visibility;
            }
            blinn_phong = u_ambientColor + texture_color + specular_illumination;
            frag_color = vec4(blinn_phong, 1.0);
        
        }
        else if (u_gammaToggle) {
            blinn_phong = pow(blinn_phong, vec3(1 / 2.2));
            frag_color = vec4(blinn_phong, 1.0);
        }
        else {
            frag_color = vec4(blinn_phong, 1.0);
        }

        if(u_texcoordToggle) {
            frag_color = vec4(v_texcoord_0, 0.0, 0.0);
        }

    }
    else {
        frag_color = vec4(v_color, 1.0);
    }
    
}

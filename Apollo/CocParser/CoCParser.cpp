#include "CoCParser.h"
#include <sstream>

using namespace std;

const int BUFFER_SIZE = 128;

namespace CoC {
    
#pragma warning( disable : 4996 )
#pragma warning( disable : 4800 )

template <typename T>
struct TextureValue {
public:
    T           value;
    string      filename;
};

struct Color4f {
    float r, g, b, a;
    Color4f() : r(0), g(0), b(0), a(0) {}
    Color4f(float r, float g, float b) : r(r), g(g), b(b), a(1.0) {}

	Apollo::Color4f ToColor() const {
		return Apollo::Color4f(r, g, b, a);
	}
};

struct CoCShader {
    enum Type {
        LAMBERT,
        PHONG,
        BLINN,
        EMITTER
    };
    int id;
    string name;
    Type type;
    TextureValue<Color4f> color;
    TextureValue<float> diffuse;
    TextureValue<Color4f> ambient;
    TextureValue<Color4f> transparent;
    TextureValue<Color4f> irradiance;
    TextureValue<float> ior;
    TextureValue<float> absorbance;
    int refractions;
    TextureValue<float> refraction_blur;
    TextureValue<int> refraction_rays;
    string normalMap;
    string bumpMap;
    TextureValue<Color4f> specular;
    TextureValue<float> spec_power;
	TextureValue<float> reflectivity;
	TextureValue<float> reflection_blur;
	TextureValue<int> reflection_rays;
    TextureValue<float> spec_eccen;
	TextureValue<float> spec_roll;
    TextureValue<float> intensity;
	int shadow_rays;
    int visible;
    float decay;
};

struct TextureData {
    string filename;
    float coverageX, coverageY;
    float translateX, translateY;
    float rotateFrame;
    int mirrorU, mirrorV;
    int stagger;
    int wrapU, wrapV;
    float repeatU, repeatV;
    float offsetU, offsetV;
    float rotateUV;
    float noiseU, noiseV;
};

struct AmbientLight {
    TextureValue<Color4f> tint;
    TextureValue<float> intensity;
    int cast_shadow;
};

struct AreaLight : public AmbientLight {
    Apollo::Vector3 tl, tr, bl, br;
    Apollo::Vector3 direction;
    int num_shadow_rays;
};

struct PointLight : public AmbientLight {
    Apollo::Vector3 position;
    int decay;
};

struct DirectionalLight : public AmbientLight {
    Apollo::Vector3 direction;
};

struct SpotLight : public AmbientLight {
    Apollo::Vector3 direction;
    Apollo::Vector3 position;
    int decay;
    float penumbra;
    float cone_angle;
};

struct MeshData {
    vector<Apollo::Vector3> vertices;
    vector<Apollo::Vector3> normals;
    vector<Apollo::UV> uvs;
    vector<Apollo::Triangle::Data> triangles;
    int shader_id;
};

struct Camera {
    string name;
    Apollo::Vector3 position;
    Apollo::Vector3 look;
    Apollo::Vector3 up;
    float fov;
    float aspect_ratio;
    float focal_length;
    Color4f bgcolor;
};

struct CoCData {
    int width, height;
    int anti_alias_samples;
    int ray_depth;
    string default_camera;

    vector<Camera> cameras;
    vector<TextureData> textures;
    vector<CoCShader> shaders;
    vector<AmbientLight> ambientLights;
    vector<AreaLight> areaLights;
    vector<PointLight> pointLights;
    vector<DirectionalLight> directionalLights;
    vector<SpotLight> spotLights;
    vector<MeshData> meshes;
};

void CoCParserException::Throw(const string& tag) {
    stringstream ss;
    ss << "Error parsing file.  Expecting tag: " << tag;
    throw CoCParserException(ss.str());
}

int LookupShaderId(CoCData* data, const char* buffer) {
    for (unsigned int i = 0; i < data->shaders.size(); ++i) {
        if (!strcmp(buffer, data->shaders[i].name.c_str()))
            return data->shaders[i].id;
    }
    CoCParserException::Throw("Invalid shader");
    return -1;
}

bool ParseInt(FILE* file, char* buffer, const char* field, int* result) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%d", result);
        return true;
    }
    return false;
}

bool ParseFloat(FILE* file, char* buffer, const char* field, float* result) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%f", result);
        return true;
    }
    return false;
}

bool ParseString(FILE* file, char* buffer, const char* field, string* result) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%s", buffer);
        *result = buffer;
        return true;
    }
    return false;
}

bool ParseColor4f(FILE* file, char* buffer, const char* field, Color4f* result) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%f %f %f", &result->r, &result->g, &result->b);
        result->a = 1;
        return true;
    }
    return false;
}

bool ParseIntMap(FILE* file, char* buffer, const char* field, const char* error, TextureValue<int>* val) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%s", buffer);
        if (!strcmp(buffer, "val")) {
            fscanf(file, "%d", &val->value);
        } else if (!strcmp(buffer, "map")) {
            fscanf(file, "%s", buffer);
            val->filename = buffer;
        } else {
            sprintf(buffer, "<%s:%s>", error, field);
            CoCParserException::Throw(buffer);
        }
        return true;
    }
    return false;
}

bool ParseFloatMap(FILE* file, char* buffer, const char* field, const char* error, TextureValue<float>* val) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%s", buffer);
        if (!strcmp(buffer, "val")) {
            fscanf(file, "%f", &val->value);
        } else if (!strcmp(buffer, "map")) {
            fscanf(file, "%s", buffer);
            val->filename = buffer;
        } else {
            sprintf(buffer, "<%s:%s>", error, field);
            CoCParserException::Throw(buffer);
        }
        return true;
    }
    return false;
}

bool ParseImageReference(FILE* file, char* buffer, const char* field, string* result) {
    if (!strcmp(buffer, field)) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "none")) {
            *result = "";
	    } else {
            *result = buffer;
	    }
		return true;
    }
    return false;
}

bool ParseColorMap(FILE* file, char* buffer, const char* field, const char* error, TextureValue<Color4f>* color) {
    if (!strcmp(buffer, field)) {
        float f1, f2, f3;
        fscanf(file, "%s", buffer);
        if (!strcmp(buffer, "val")) {
            fscanf(file, "%f %f %f", &f1, &f2, &f3);
            color->value = Color4f(f1, f2, f3);
        } else if (!strcmp(buffer, "map")) {
            fscanf(file, "%s", buffer);
            color->filename = buffer;
        } else {
            sprintf(buffer, "<%s:%s>", error, field);
            CoCParserException::Throw(buffer);
        }
        return true;
    }
    return false;
}

bool ParseUV(FILE* file, char* buffer, const char* field, float* u, float* v) {
    if (!strcmp(buffer, field)) {
        fscanf(file, "%f %f", u, v);
        return true;
    }
    return false;
}

bool ParseVector3(FILE* file, char* buffer, const char* field, Apollo::Vector3* result) {
    if (!strcmp(buffer, field)) {
        float f1, f2, f3;
        fscanf(file, "%f %f %f", &f1, &f2, &f3);
        result->x = f1;
        result->y = f2;
        result->z = f3;
        return true;
    }
    return false;
}

void ParseGeometry(FILE* file, CoCData* data) {
    char buffer[BUFFER_SIZE];
    int shader_id = -1;
    vector<Apollo::Vector3> vertices;
    vector<Apollo::Vector3> normals;
    vector<Apollo::UV> uvs;
    vector<Apollo::Triangle::Data> triangles;
    bool was_face = false;

    int vertex_offset = 1;
    int normal_offset = 1;
    int uv_offset = 1;
    Apollo::Vector3 v;
    Apollo::UV uv;
    Apollo::Triangle::Data triangle;

    while (fscanf(file, "%s", buffer) != -1) {
	    // skip comments starting with '#'
	    if (buffer[0] == '#') {
	        while ((char)getc(file) != '\n')
                ;
	        continue;
	    }

        if (ParseVector3(file, buffer, "vn", &v)) {
            normals.push_back(v);
        } else if (!strcmp(buffer, "vt")) {
            fscanf(file, "%f %f", &uv.u, &uv.v);
            uvs.push_back(uv);
        } else if (ParseVector3(file, buffer, "v", &v)) {
            if (was_face) {
                if (shader_id == -1) {
                    CoCParserException::Throw("No shader specified");
                }
                data->meshes.push_back(MeshData());
                MeshData& meshData = data->meshes[data->meshes.size() - 1];
                meshData.shader_id = shader_id;
                meshData.vertices = vertices;
                meshData.normals = normals;
                meshData.uvs = uvs;
                meshData.triangles = triangles;

                was_face = false;
                vertex_offset += vertices.size();
                normal_offset += normals.size();
                uv_offset += uvs.size();

                vertices.clear();
                normals.clear();
                uvs.clear();
                triangles.clear();
            }
            vertices.push_back(v);
        } else if (!strcmp(buffer, "f")) {
            was_face = true;
	        int num;
	        if ((num = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d", 
                &triangle.A, &triangle.tA, &triangle.nA,  
                &triangle.B, &triangle.tB, &triangle.nB, 
                &triangle.C, &triangle.tC, &triangle.nC)) == 9) {

                triangle.A -= vertex_offset; triangle.B -= vertex_offset; triangle.C -= vertex_offset;
                triangle.tA -= uv_offset; triangle.tB -= uv_offset; triangle.tC -= uv_offset;
                triangle.nA -= normal_offset; triangle.nB -= normal_offset; triangle.nC -= normal_offset;
                triangles.push_back(triangle);
                continue;
	        } else if (num == 1) {
		        if (fscanf(file, "%d %d", &triangle.B, &triangle.C) == 2) {
                    --triangle.A; --triangle.B; --triangle.C;
                    triangle.nA = triangle.nB = triangle.nC = -1;
                    triangle.tA = triangle.tB = triangle.tC = -1;
                    triangles.push_back(triangle);
                    Apollo::ApolloException::NotYetImplemented();
                    continue;
		        } 
            }
		    CoCParserException::Throw("<geometry: face>");
	    } else if (!strcmp(buffer, "usemtl")) {
	        fscanf(file, "%s", buffer);
            shader_id = LookupShaderId(data, buffer);
	    }
    }

    if (triangles.size() > 0) {
        if (shader_id == -1) {
            CoCParserException::Throw("No shader specified");
        }
        data->meshes.push_back(MeshData());
        MeshData& meshData = data->meshes[data->meshes.size() - 1];
        meshData.shader_id = shader_id;
        meshData.vertices = vertices;
        meshData.normals = normals;
        meshData.uvs = uvs;
        meshData.triangles = triangles;
    }
}

void ParseEmitter(FILE* file, CoCData* data, int id) {
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    CoCShader shader;
    shader.type = CoCShader::EMITTER;
    shader.id = id;
    shader.name = buffer;
    while (true) {
	    fscanf(file, "%s", buffer);
        if (ParseInt(file, buffer, "visible", &shader.visible))
            continue;
	    else if (ParseColorMap(file, buffer, "color", "Material: Emitter", &shader.color))
            continue;
	    else if (ParseFloatMap(file, buffer, "intensity", "Material: Emitter", &shader.intensity))
            continue;
        else if (ParseFloat(file, buffer, "decay", &shader.decay))
            continue;
	    else if (ParseInt(file, buffer, "shadow_rays", &shader.shadow_rays))
            continue;
	    else if (!strcmp(buffer, "<end>")) {
            data->shaders.push_back(shader);
            return;
        }
	    else if (buffer[0] != '#') 
	        CoCParserException::Throw("<Material: Emitter>");
	    while (fgetc(file) != '\n')
	        ;
    }
    CoCParserException::Throw("<Material: Emitter>");
}

void ParseLambert(FILE* file, CoCData* data, int id) {
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    CoCShader shader;
    shader.type = CoCShader::LAMBERT;
    shader.id = id;
    shader.name = buffer;
    while (true) {
	    fscanf(file, "%s", buffer);
        if (ParseColorMap(file, buffer, "color", "Material: Lambert", &shader.color))
            continue;
        else if (ParseColorMap(file, buffer, "ambient_color", "Material: Lambert", &shader.ambient))
            continue;
        else if (ParseFloatMap(file, buffer, "diffuse", "Material: Lambert", &shader.diffuse))
            continue;
        else if (ParseColorMap(file, buffer, "transparency", "Material: Lambert", &shader.transparent))
            continue;
        else if (ParseColorMap(file, buffer, "irradiance", "Material: Lambert", &shader.irradiance))
            continue;
        else if (ParseImageReference(file, buffer, "bump_map", &shader.bumpMap))
            continue;
        else if (ParseImageReference(file, buffer, "normal_map", &shader.normalMap))
            continue;
        else if (ParseInt(file, buffer, "refractions", &shader.refractions)) 
            continue;
        else if (ParseFloatMap(file, buffer, "refractive_index", "Material: Lambert", &shader.ior))
            continue;
        else if (ParseFloatMap(file, buffer, "light_absorbance", "Material: Lambert", &shader.absorbance))
            continue;
        else if (ParseFloatMap(file, buffer, "refraction_blur", "Material: Lambert", &shader.refraction_blur))
            continue;
        else if (ParseIntMap(file, buffer, "refraction_rays", "Material: Lambert", &shader.refraction_rays))
            continue;
	    else if (!strcmp(buffer, "<end>")) {
            data->shaders.push_back(shader);
	        return;
        } else if (buffer[0] != '#')
	        CoCParserException::Throw("<Material: Lambert>");
	    while (fgetc(file) != '\n')
	        ;		
    }
    CoCParserException::Throw("<Material: Lambert>");
} 

void ParsePhong(FILE* file,  CoCData* data, int id) {
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    CoCShader shader;
    shader.type = CoCShader::PHONG;
    shader.id = id;
    shader.name = buffer;
    while (true) {
	    fscanf(file, "%s", buffer);
        if (ParseColorMap(file, buffer, "color", "Material: Phong", &shader.color))
            continue;
        else if (ParseColorMap(file, buffer, "ambient_color", "Material: Phong", &shader.ambient))
            continue;
        else if (ParseFloatMap(file, buffer, "diffuse", "Material: Phong", &shader.diffuse))
            continue;
        else if (ParseColorMap(file, buffer, "transparency", "Material: Phong", &shader.transparent))
            continue;
        else if (ParseColorMap(file, buffer, "irradiance", "Material: Phong", &shader.irradiance))
            continue;
        else if (ParseImageReference(file, buffer, "bump_map", &shader.bumpMap))
            continue;
        else if (ParseImageReference(file, buffer, "normal_map", &shader.normalMap))
            continue;
        else if (ParseInt(file, buffer, "refractions", &shader.refractions)) 
            continue;
        else if (ParseFloatMap(file, buffer, "spec_cosine_power", "Material: Phong", &shader.spec_power))
            continue;
        else if (ParseColorMap(file, buffer, "spec_color", "Material: Phong", &shader.specular))
            continue;
        else if (ParseFloatMap(file, buffer, "reflectivity", "Material: Phong", &shader.reflectivity))
            continue;
        else if (ParseFloatMap(file, buffer, "refractive_index", "Material: Phong", &shader.ior))
            continue;
        else if (ParseFloatMap(file, buffer, "light_absorbance", "Material: Phong", &shader.absorbance))
            continue;
        else if (ParseFloatMap(file, buffer, "refraction_blur", "Material: Phong", &shader.refraction_blur))
            continue;
        else if (ParseIntMap(file, buffer, "refraction_rays", "Material: Phong", &shader.refraction_rays))
            continue;
        else if (ParseFloatMap(file, buffer, "reflection_blur", "Material: Phong", &shader.reflection_blur))
            continue;
        else if (ParseIntMap(file, buffer, "reflection_rays", "Material: Phong", &shader.reflection_rays))
            continue;
	    else if (!strcmp(buffer, "<end>")) {
            data->shaders.push_back(shader);
            return;
        } else if (buffer[0] != '#') 
	        CoCParserException::Throw("<Material: Phong>");
	    while (fgetc(file) != '\n')
	        ;	
    }
    CoCParserException::Throw("<Material: Phong>");
} 

void ParseBlinn(FILE* file, CoCData* data, int id) {
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    CoCShader shader;
    shader.type = CoCShader::BLINN;
    shader.id = id;
    shader.name = buffer;
    while (true) {
	    fscanf(file, "%s", buffer);
        if (ParseColorMap(file, buffer, "color", "Material: Blinn", &shader.color))
            continue;
        else if (ParseColorMap(file, buffer, "ambient_color", "Material: Blinn", &shader.ambient))
            continue;
        else if (ParseFloatMap(file, buffer, "diffuse", "Material: Blinn", &shader.diffuse))
            continue;
        else if (ParseColorMap(file, buffer, "transparency", "Material: Blinn", &shader.transparent))
            continue;
        else if (ParseColorMap(file, buffer, "irradiance", "Material: Blinn", &shader.irradiance))
            continue;
        else if (ParseImageReference(file, buffer, "bump_map", &shader.bumpMap))
            continue;
        else if (ParseImageReference(file, buffer, "normal_map", &shader.normalMap))
            continue;
        else if (ParseInt(file, buffer, "refractions", &shader.refractions)) 
            continue;
        else if (ParseFloatMap(file, buffer, "spec_eccentricity", "Material: Blinn", &shader.spec_eccen))
            continue;
        else if (ParseFloatMap(file, buffer, "spec_rolloff", "Material: Blinn", &shader.spec_roll))
            continue;
        else if (ParseColorMap(file, buffer, "spec_color", "Material: Blinn", &shader.specular))
            continue;
        else if (ParseFloatMap(file, buffer, "reflectivity", "Material: Blinn", &shader.reflectivity))
            continue;
        else if (ParseFloatMap(file, buffer, "refractive_index", "Material: Blinn", &shader.ior))
            continue;
        else if (ParseFloatMap(file, buffer, "light_absorbance", "Material: Blinn", &shader.absorbance))
            continue;
        else if (ParseFloatMap(file, buffer, "refraction_blur", "Material: Blinn", &shader.refraction_blur))
            continue;
        else if (ParseIntMap(file, buffer, "refraction_rays", "Material: Blinn", &shader.refraction_rays))
            continue;
        else if (ParseFloatMap(file, buffer, "reflection_blur", "Material: Blinn", &shader.reflection_blur))
            continue;
        else if (ParseIntMap(file, buffer, "reflection_rays", "Material: Blinn", &shader.reflection_rays))
            continue;
	    else if (!strcmp(buffer, "<end>")) {
            data->shaders.push_back(shader);
            return;
        } else if (buffer[0] != '#')
	        CoCParserException::Throw("<Material: Blinn>");
	    while (fgetc(file) != '\n')
	        ;	
    }
} 

void ParseAreaLight(FILE* file, CoCData* data) { 
    AreaLight light;
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "tl", &light.tl))
        CoCParserException::Throw("<AreaLight: tl>");
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "tr", &light.tr))
        CoCParserException::Throw("<AreaLight: tr>");
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "br", &light.br))
        CoCParserException::Throw("<AreaLight: br>");
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "bl", &light.bl))
        CoCParserException::Throw("<AreaLight: bl>");

    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "direction", &light.direction))
        CoCParserException::Throw("<AreaLight: bl>");
    fscanf(file, "%s", buffer);
    if (!ParseColorMap(file, buffer, "tint", "AreaLight", &light.tint))
        CoCParserException::Throw("<AreaLight: tint>");
    fscanf(file, "%s", buffer);
    if (!ParseFloatMap(file, buffer, "intensity", "AreaLight", &light.intensity))
	    CoCParserException::Throw("<AreaLight: intensity>");
    fscanf(file, "%s", buffer);
    if (!ParseInt(file, buffer, "shadowrays", &light.num_shadow_rays))
        return;
    fscanf(file, "%s", buffer);
    ParseInt(file, buffer, "casts_shadow", &light.cast_shadow);
    data->areaLights.push_back(light);
}

void ParsePointLight(FILE* file, CoCData* data) { 
    PointLight light;
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "pos", &light.position))
        CoCParserException::Throw("<PointLight: pos>");
    fscanf(file, "%s", buffer);
    if (!ParseColorMap(file, buffer, "tint", "Pointight", &light.tint))
        CoCParserException::Throw("<PointLight: tint>");
    fscanf(file, "%s", buffer);
    if (!ParseFloatMap(file, buffer, "intensity", "PointLight", &light.intensity))
	    CoCParserException::Throw("<PointLight: intensity>");
    fscanf(file, "%s", buffer);
    if (!ParseInt(file, buffer, "decay", &light.decay))
	    CoCParserException::Throw("<PointLight: decay>");
    fscanf(file, "%s", buffer);
    ParseInt(file, buffer, "casts_shadow", &light.cast_shadow);
    data->pointLights.push_back(light);
}

void ParseDirectionalLight(FILE* file, CoCData* data) { 
    DirectionalLight light;
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "direction", &light.direction))
        CoCParserException::Throw("<DirectionalLight: bl>");
    fscanf(file, "%s", buffer);
    if (!ParseColorMap(file, buffer, "tint", "DirectionalLight", &light.tint))
        CoCParserException::Throw("<DirectionalLight: tint>");
    fscanf(file, "%s", buffer);
    if (!ParseFloatMap(file, buffer, "intensity", "DirectionalLight", &light.intensity))
	    CoCParserException::Throw("<DirectionalLight: intensity>");
    fscanf(file, "%s", buffer);
    ParseInt(file, buffer, "casts_shadow", &light.cast_shadow);
    data->directionalLights.push_back(light);
}

void ParseAmbientLight(FILE* file, CoCData* data) { 
    AmbientLight light; 
    char buffer[BUFFER_SIZE];
    fscanf(file, "%s", buffer);
    if (!ParseColorMap(file, buffer, "tint", "Pointight", &light.tint))
        CoCParserException::Throw("<AmbientLight: tint>");
    fscanf(file, "%s", buffer);
    if (!strcmp(buffer, "intensity")) 
        fscanf(file, "%f", &light.intensity);
    light.cast_shadow = false;
    data->ambientLights.push_back(light);
}

void ParseSpotLight(FILE* file, CoCData* data) { 
    SpotLight light;
    char buffer[BUFFER_SIZE];

    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "pos", &light.position))
        CoCParserException::Throw("<SpotLight: pos>");
    fscanf(file, "%s", buffer);
    if (!ParseVector3(file, buffer, "direction", &light.direction))
        CoCParserException::Throw("<SpotLight: bl>");
    fscanf(file, "%s", buffer);
    if (!ParseColorMap(file, buffer, "tint", "SpotLight", &light.tint))
        CoCParserException::Throw("<SpotLight: tint>");
    fscanf(file, "%s", buffer);
    if (!ParseFloatMap(file, buffer, "intensity", "SpotLight", &light.intensity))
	    CoCParserException::Throw("<SpotLight: intensity>");
    fscanf(file, "%s", buffer);
    if (!ParseInt(file, buffer, "decay", &light.decay))
	    CoCParserException::Throw("<SpotLight: decay>");
    fscanf(file, "%s", buffer);
    if (!ParseFloat(file, buffer, "cone_angle", &light.cone_angle))
	    CoCParserException::Throw("<SpotLight: cone_angle>");
    fscanf(file, "%s", buffer);
    if (!ParseFloat(file, buffer, "penumbra", &light.penumbra))
	    CoCParserException::Throw("<SpotLight: penumbra>");
    fscanf(file, "%s", buffer);
    ParseInt(file, buffer, "casts_shadow", &light.cast_shadow);
    data->spotLights.push_back(light);
}

void ParseLight(FILE* file, CoCData* data) {
    char buffer[BUFFER_SIZE];
    /* parse lights */
    while (true) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "type")) {
	        fscanf(file, "%s", buffer);
	        if (!strcmp(buffer, "POINT")) {
		        ParsePointLight(file, data);
	        } else if(!strcmp(buffer, "SPOT")) {
		        ParseSpotLight(file, data);
	        } else if(!strcmp(buffer, "DIRECTIONAL")) {
		        ParseDirectionalLight(file, data);
	        } else if(!strcmp(buffer, "AMBIENT")) {
		        ParseAmbientLight(file, data);
	        } else if(!strcmp(buffer, "AREA")) {
		        ParseAreaLight(file, data);
	        }
	    } else if (!strcmp(buffer, "<geometry>")) {
	        break;
	    }
	    else if (buffer[0] != '#') {
	        CoCParserException::Throw("<lights>");
        }
	    while (fgetc(file) != '\n')
	        ;
    } 
    ParseGeometry(file, data);
}

/*
void ParsePhotonMapping(FILE* file, RayTracer* tracer) {
    tracer->tracerParams.photon_mapping = true;

    tracer->photonMappingParams.caustic_photons = 50000;
    tracer->photonMappingParams.indirect_photons = 50000;
    tracer->photonMappingParams.shadow_photons = 0;
    tracer->photonMappingParams.max_caustic_photons = 100000;
    tracer->photonMappingParams.max_indirect_photons = 100000;
    tracer->photonMappingParams.caustic_gather_radius = 5;
    tracer->photonMappingParams.indirect_gather_radius = 5;
    tracer->photonMappingParams.caustic_gather_photons = 100;
    tracer->photonMappingParams.indirect_gather_photons = 100;
    tracer->photonMappingParams.shadow_radius = 1;

    char buffer[BUFFER_SIZE];
    int num_cameras;

    while (true) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "caustic_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.caustic_photons);
	    else if (!strcmp(buffer, "indirect_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.indirect_photons);
	    else if (!strcmp(buffer, "shadow_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.shadow_photons);
	    else if (!strcmp(buffer, "max_shadow_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.max_shadow_photons);
	    else if (!strcmp(buffer, "max_caustic_photons")) 
	        fscanf(file, "%d", &tracer->photonMappingParams.max_caustic_photons);
	    else if (!strcmp(buffer, "max_indirect_photons")) 
	        fscanf(file, "%d", &tracer->photonMappingParams.max_indirect_photons);
	    else if (!strcmp(buffer, "caustic_gather_radius"))
	        fscanf(file, "%f", &tracer->photonMappingParams.caustic_gather_radius);
	    else if (!strcmp(buffer, "indirect_gather_radius"))
	        fscanf(file, "%f", &tracer->photonMappingParams.indirect_gather_radius);
	    else if (!strcmp(buffer, "shadow_radius"))
	        fscanf(file, "%f", &tracer->photonMappingParams.shadow_radius);
	    else if (!strcmp(buffer, "caustic_gather_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.caustic_gather_photons);
	    else if (!strcmp(buffer, "indirect_gather_photons"))
	        fscanf(file, "%d", &tracer->photonMappingParams.indirect_gather_photons);
	    else if (!strcmp(buffer, "final_gather_rays"))
	        fscanf(file, "%d", &tracer->tracerParams.final_gather_rays);
	    else if (!strcmp(buffer, "<cameras")) {
	        fscanf(file, "%4s", buffer);
	        fscanf(file, "%d", &num_cameras);
	        fscanf(file, "%s", buffer);
	        ParseCamera(file, tracer, num_cameras);
	    }
	    else if (buffer[0] != '#')
	        CoCParserException::Throw("<photon_mapping>");
	    else {
	        while (fgetc(file) != '\n')
		    ;
	    }
    }

    tracer->photonMappingParams.direct_photons = tracer->photonMappingParams.shadow_photons;
    if (tracer->photonMappingParams.shadow_photons > 0) {
	    tracer->tracerParams.shadow_photons = true;
    }
}
*/

void ParseMaterials(FILE* file, CoCData* data) {
    int shader_id = 0;
    int num_lights;
    char buffer[BUFFER_SIZE];

    while (true) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "newmtl")) {
	        fscanf(file, "%s", buffer);
	        if (!strcmp(buffer, "[Lambert]"))
		        ParseLambert(file, data, shader_id++);
	        else if (!strcmp(buffer, "[Phong]"))
		        ParsePhong(file, data, shader_id++);
	        else if (!strcmp(buffer, "[Blinn]"))
		        ParseBlinn(file, data, shader_id++);
	        else if (!strcmp(buffer, "[Emitter]")) 
		        ParseEmitter(file, data, shader_id++);
	    } else if (!strcmp(buffer, "<lights")) {
	        fscanf(file, "%4s", buffer);
	        fscanf(file, "%d", &num_lights);
	        fscanf(file, "%s", buffer);
	        break;
	    } else if (!strcmp(buffer, "<geometry>")) {
	        return ParseGeometry(file, data);
	    } else if (buffer[0] != '#') {
	        CoCParserException::Throw("<materials>");
        }

	    while (fgetc(file) != '\n')
	        ;		
    }
    ParseLight(file, data);
}

void ParseTextures(FILE* file, CoCData* data, int num_textures) {
    char buffer[BUFFER_SIZE];
    data->textures.resize(num_textures);

    for (int i = 0; i < num_textures; ++i) {
        TextureData* img = &data->textures[i];
	    fscanf(file, "%s", buffer);
        img->filename = buffer;
	    fscanf(file, "%s", buffer);
        if (!ParseUV(file, buffer, "Coverage", &img->coverageX, &img->coverageY))
	        CoCParserException::Throw("<texture: Coverage>");
	    fscanf(file, "%s", buffer);
        if (!ParseUV(file, buffer, "TranslateFrame", &img->translateX, &img->translateY))
	        CoCParserException::Throw("<texture: TranslateFrame>");
	    fscanf(file, "%s", buffer);
        if (!ParseFloat(file, buffer, "RotateFrame", &img->rotateFrame))
	        CoCParserException::Throw("<texture: RotateFrame>");
	    fscanf(file, "%s", buffer);
        if (!ParseInt(file, buffer, "MirrorU", &img->mirrorU))
	        CoCParserException::Throw("<texture: MirrorU>");
	    fscanf(file, "%s", buffer);
        if (!ParseInt(file, buffer, "MirrorV", &img->mirrorV))
	        CoCParserException::Throw("<texture: MirrorV>");
	    fscanf(file, "%s", buffer);
        if (!ParseInt(file, buffer, "Stagger", &img->stagger))
	        CoCParserException::Throw("<texture: Stagger>");
	    fscanf(file, "%s", buffer);
        if (!ParseInt(file, buffer, "WrapU", &img->wrapU))
	        CoCParserException::Throw("<texture: WrapU>");
	    fscanf(file, "%s", buffer);
        if (!ParseInt(file, buffer, "WrapV", &img->wrapV))
	        CoCParserException::Throw("<texture: WrapV>");
	    fscanf(file, "%s", buffer);
        if (!ParseUV(file, buffer, "RepeatUV", &img->repeatU, &img->repeatV))
	        CoCParserException::Throw("<texture: RepeatUV>");
	    fscanf(file, "%s", buffer);
        if (!ParseUV(file, buffer, "OffsetUV", &img->offsetU, &img->offsetV))
	        CoCParserException::Throw("<texture: OffsetUV>");
	    fscanf(file, "%s", buffer);
        if (!ParseFloat(file, buffer, "RotateUV", &img->rotateUV))
	        CoCParserException::Throw("<texture: RotateUV>");
	    fscanf(file, "%s", buffer);
        if (!ParseUV(file, buffer, "NoiseUV", &img->noiseU, &img->noiseV))
	        CoCParserException::Throw("<texture: NoiseUV>");
    }

    int num_mats;

    while (true) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "<materials")) {
	        fscanf(file, "%4s", buffer);
	        fscanf(file, "%d", &num_mats);
	        fscanf(file, "%s", buffer);
	        break;
	    } else if (buffer[0] != '#') {
	        CoCParserException::Throw("<textures>");
        }

	    while (fgetc(file) != '\n')
	        ;		
    }

    ParseMaterials(file, data);
}

void ParseCameras(FILE* file, CoCData* data, int num_cameras) {
    int curCam = -1;
    data->cameras.resize(num_cameras);
    Camera* camera = nullptr;
    int num_textures;
    char buffer[BUFFER_SIZE];

    /* parse cameras */
    while (true) {
	    fscanf(file, "%s", buffer);
	    if (!strcmp(buffer, "name")) {
	        ++curCam;
            camera = &data->cameras[curCam];
	        fscanf(file, "%s", buffer);
            camera->name = buffer;
	        fscanf(file, "%s", buffer);
        } 
        if (camera == nullptr) 
            CoCParserException::Throw("<camera>");
        if (ParseVector3(file, buffer, "pos", &camera->position)) 
            continue;
        if (ParseVector3(file, buffer, "look", &camera->look)) 
            continue;
        if (ParseVector3(file, buffer, "up", &camera->up))
            continue;
        if (ParseFloat(file, buffer, "focal_length", &camera->focal_length))
            continue;
        if (ParseFloat(file, buffer, "fov", &camera->fov))
            continue;
        if (ParseFloat(file, buffer, "aspect_ratio", &camera->aspect_ratio))
            continue;
        if (ParseColor4f(file, buffer, "bgcolor", &camera->bgcolor))
            continue;
        if (!strcmp(buffer, "<textures")) {
	        fscanf(file, "%4s", buffer);
	        fscanf(file, "%d", &num_textures);
	        fscanf(file, "%s", buffer);
	        break;
	    } else if (buffer[0] != '#') {
            CoCParserException::Throw("<camera>");
        }
	    while (fgetc(file) != '\n')
	        ;
    }

    ParseTextures(file, data, num_textures);
}

void ParseMetadata(FILE* file, CoCData* data) {
    char buffer[BUFFER_SIZE];
    while (true) {
        fscanf(file, "%s", buffer);
        if (ParseInt(file, buffer, "width", &data->width))
            continue;
        if (ParseInt(file, buffer, "height", &data->height))
            continue;
        if (ParseInt(file, buffer, "ray_depth", &data->ray_depth))
            continue;
        if (ParseInt(file, buffer, "anti_alias", &data->anti_alias_samples))
            continue;
        if (ParseString(file, buffer, "defcam", &data->default_camera))
            continue;
        if (!strcmp(buffer, "<photon_mapping>")) {
            Apollo::ApolloException::NotYetImplemented();
        } else if (!strcmp(buffer, "<cameras")) {
            int num_cameras;
            fscanf(file, "%4s", buffer);
            fscanf(file, "%d", &num_cameras);
            fscanf(file, "%s", buffer);
            ParseCameras(file, data, num_cameras);
            return;
        } else if (buffer[0] != '#') {
            CoCParserException::Throw("<meta>");
        } else {
            while (fgetc(file) != '\n')
                ;
        }
    }
}

void ConvertToApolloScene(CoCData* data, Apollo::Scene* scene) {
    Apollo::PoolAllocator* allocator = scene->GetAllocator();

    // Set camera
    Apollo::PerspectiveCamera* camera = new Apollo::PerspectiveCamera();
    allocator->Add(camera);
    scene->SetCamera(camera);
    camera->SetPosition(data->cameras[0].position);
    camera->Orient(data->cameras[0].look, data->cameras[0].up);
    camera->SetFov(data->cameras[0].fov);
    camera->SetAspectRatio(data->cameras[0].aspect_ratio);
    camera->SetImageDimension(data->width, data->height);

    // Set lights
	for (size_t i = 0; i < data->pointLights.size(); ++i) {
		const PointLight& point_light = data->pointLights[i];
		Apollo::Light* light = Apollo::Light::CreatePointLight(point_light.position, 
			point_light.tint.value.ToColor(), point_light.intensity.value);
		light->SetCastShadow((bool)point_light.cast_shadow);
        light->SetLightFalloff(point_light.decay);
		scene->AddLight(light);
	}

	// TODO - Add spot lights
	for (size_t i = 0; i < data->spotLights.size(); ++i) {
		const SpotLight& spot_light = data->spotLights[i];
		Apollo::Light* light = Apollo::Light::CreatePointLight(spot_light.position, 
			spot_light.tint.value.ToColor(), spot_light.intensity.value);
		light->SetCastShadow((bool)spot_light.cast_shadow);
        light->SetLightFalloff(spot_light.decay);
		scene->AddLight(light);
	}

	// TODO - Add directional lights
	for (size_t i = 0; i < data->directionalLights.size(); ++i) {
		const DirectionalLight& directional_light = data->directionalLights[i];
		Apollo::Vector3 pos = -directional_light.direction;
		pos *= 10000;
		Apollo::Light* light = Apollo::Light::CreatePointLight(pos,
			directional_light.tint.value.ToColor(), directional_light.intensity.value);
		light->SetCastShadow((bool)directional_light.cast_shadow);
		scene->AddLight(light);
	}
	
	for (size_t i = 0; i < data->ambientLights.size(); ++i) {
		Apollo::Light* light = Apollo::Light::CreateAmbientLight(
			data->ambientLights[i].tint.value.ToColor(), data->ambientLights[i].intensity.value);
		scene->AddLight(light);
	}

    // Add Geometry
    for (unsigned int i = 0; i < data->meshes.size(); ++i) {
        MeshData& meshData = data->meshes[i];
        Apollo::Mesh* mesh = new Apollo::Mesh(meshData.vertices, meshData.normals, meshData.uvs, meshData.triangles);
        allocator->Add(mesh);
        for (unsigned int i = 0; i < data->shaders.size(); ++i) {
            if (meshData.shader_id == data->shaders[i].id) {
                CoCShader& shaderData = data->shaders[i];
                Apollo::Color4f color;
                if (shaderData.type == CoCShader::EMITTER) {
                    color.r = shaderData.color.value.r * shaderData.intensity.value;
                    color.g = shaderData.color.value.g * shaderData.intensity.value;
                    color.b = shaderData.color.value.b * shaderData.intensity.value;
                    color.a = 1;
                } else {
                    color.r = shaderData.color.value.r;
                    color.g = shaderData.color.value.g;
                    color.b = shaderData.color.value.b;
                    color.a = shaderData.color.value.a;
                }
                Apollo::LambertianShader* shader = new Apollo::LambertianShader(color);
                allocator->Add(shader);
                mesh->SetShader(shader);
                break;
            }
        }
        assert(mesh->GetShader() != nullptr);
        scene->AddModel(mesh);
    }
}

void CoCParser::Parse(const char* filename, Apollo::Scene* scene) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        stringstream ss;
        ss << "Invalid file: " << filename;
        throw CoCParserException(ss.str());
    }
    char buffer[BUFFER_SIZE];
    CoCData data;
    while (true) {
        fscanf(file, "%s", buffer);
        if (!strcmp(buffer, "<meta>")) {
            break;
        }
        if (buffer[0] != '#') {
            CoCParserException::Throw("<meta>");
        }
        while (fgetc(file) != '\n')
            ;
    }
    ParseMetadata(file, &data);

    ConvertToApolloScene(&data, scene);
}

}

#pragma warning( default : 4996 )

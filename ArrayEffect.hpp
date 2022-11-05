#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>

/**
 * The Array effect repeats its input geometry several times,
 * applying a transform between each copy.
 */
class ArrayEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "Array";
	}
protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput)
		.RequestCornerAttribute(
			"uv0",
			2, MfxAttributeType::Float,
			MfxAttributeSemantic::TextureCoordinate,
			false /* not mandatory */
		);
		
		AddInput(kOfxMeshMainOutput);

		// Number of copies
		AddParam("count", 2)
		.Label("Count")
		.Range(0, 2147483647);
		
		// Translation added at each copy
		AddParam("translation", double3{0.0, 0.0, 0.0})
		.Label("Translation")
		.Range(double3{-10.0, -10.0, -10.0}, double3{10.0, 10.0, 10.0});
		AddParam("offset_uv", false)
		.Label("Offset UVs");

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle) override {
		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMeshProps input_props;
		input_mesh.FetchProperties(input_props);
		
		int count = GetParam<int>("count").GetValue();
		auto translation = to_float3(GetParam<double3>("translation").GetValue());
		bool offset_uv = GetParam<bool>("offset_uv").GetValue();

		MfxAttributeProps input_uv;
		if (offset_uv && input_mesh.HasCornerAttribute("uv0")) {
			input_mesh.GetCornerAttribute("uv0").FetchProperties(input_uv);
			if (input_uv.componentCount < 2 || input_uv.type != MfxAttributeType::Float) {
				offset_uv = false; // incompatible type, so we deactivate this feature
			}
		} else {
			offset_uv = false; // no uv available, so here again we deactivate this feature
		}

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();
		
		if (offset_uv) {
			output_mesh.AddCornerAttribute(
				"uv0",
				2, MfxAttributeType::Float,
				MfxAttributeSemantic::TextureCoordinate
			);
		}

		
		output_mesh.Allocate(
			input_props.pointCount * count,
			input_props.cornerCount * count,
			input_props.faceCount * count,
			input_props.noLooseEdge,
			input_props.constantFaceSize);

		MfxAttributeProps input_positions, input_corner_points, input_face_size;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(input_positions);
		input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(input_corner_points);
		input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(input_face_size);
		
		MfxAttributeProps output_positions, output_corner_points, output_face_counts;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(output_positions);
		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(output_corner_points);
		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(output_face_counts);
		for (int k = 0 ; k < count ; ++k) {
			for (int i = 0 ; i < input_props.pointCount ; ++i) {
				int j = i + k * input_props.pointCount;
				float *in_p = reinterpret_cast<float*>(input_positions.data + i * input_positions.stride);
				float *out_p = reinterpret_cast<float*>(output_positions.data + j * output_positions.stride);
				out_p[0] = in_p[0] + translation[0] * k;
				out_p[1] = in_p[1] + translation[1] * k;
				out_p[2] = in_p[2] + translation[2] * k;
			}

			for (int i = 0 ; i < input_props.cornerCount ; ++i) {
				int j = i + k * input_props.cornerCount;
				int *in_p = reinterpret_cast<int*>(input_corner_points.data + i * input_corner_points.stride);
				int *out_p = reinterpret_cast<int*>(output_corner_points.data + j * output_corner_points.stride);
				out_p[0] = in_p[0] + k * input_props.pointCount;
			}

			if (input_props.constantFaceSize < 0) {
				for (int i = 0 ; i < input_props.faceCount ; ++i) {
					int j = i + k * input_props.faceCount;
					int *in_p = reinterpret_cast<int*>(input_face_size.data + i * input_face_size.stride);
					int *out_p = reinterpret_cast<int*>(output_face_counts.data + j * output_face_counts.stride);
					out_p[0] = in_p[0];
				}

			}
		}
		if (offset_uv) {
			MfxAttributeProps output_uv;
			output_mesh.GetCornerAttribute("uv0").FetchProperties(output_uv);
		
			for (int k = 0 ; k < count ; ++k) {
				for (int i = 0 ; i < input_props.cornerCount ; ++i) {
					int j = i + k * input_props.cornerCount;
					float *in_p = reinterpret_cast<float*>(input_uv.data + i * input_uv.stride);
					float *out_p = reinterpret_cast<float*>(output_uv.data + j * output_uv.stride);
					out_p[0] = in_p[0] + k;
					out_p[1] = in_p[1];
				}

			}
		}

		
		output_mesh.Release();
		input_mesh.Release();
		return kOfxStatOK;

	}
};

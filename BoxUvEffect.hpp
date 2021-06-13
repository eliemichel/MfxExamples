#include <PluginSupport/MfxEffect>
#include <iostream>

/**
 * Set UVs by projecting on the faces of a box
 */
class BoxUvEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "Box UV";
	}
protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput);

		AddInput("box")
		.Label("Box Projector")
		.RequestTransform();
		
		AddInput(kOfxMeshMainOutput);

		// Position of the box projector
		AddParam("translation", double3{0.0, 0.0, 0.0})
		.Label("Box Translation")
		.Range(double3{-10.0, -10.0, -10.0}, double3{10.0, 10.0, 10.0});

		// Scale added at each copy
		AddParam("scale", double3{0.0, 0.0, 0.0})
		.Label("Box Scale");

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle) override {
		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();

		MfxMeshProps input_props;
		input_mesh.FetchProperties(input_props);
		
		auto translation = to_float3(GetParam<double3>("translation").GetValue());
		auto scale = to_float3(GetParam<double3>("scale").GetValue());

		MfxMesh box_input = GetInput("box").GetMesh();
		if (box_input.IsValid()) {
			double* matrix;
			box_input.FetchTransform(&matrix);

			translation[0] = static_cast<float>(matrix[3]);
			translation[1] = static_cast<float>(matrix[7]);
			translation[2] = static_cast<float>(matrix[11]);
		}
		
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.ForwardFrom(input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition));

		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));
		
		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
			.ForwardFrom(input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize));
		
		output_mesh.AddCornerAttribute(
			"uv0",
			2, MfxAttributeType::Float,
			MfxAttributeSemantic::TextureCoordinate
		);

		output_mesh.Allocate(
			input_props.pointCount,
			input_props.cornerCount,
			input_props.faceCount,
			input_props.noLooseEdge,
			input_props.constantFaceSize);

		MfxAttributeProps input_positions, input_corner_points;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(input_positions);
		input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(input_corner_points);
		
		MfxAttributeProps output_uv;
		output_mesh.GetCornerAttribute("uv0").FetchProperties(output_uv);
	
		for (int i = 0 ; i < input_props.cornerCount ; ++i) {
			int *in_pt = reinterpret_cast<int*>(input_corner_points.data + i * input_corner_points.stride);
			float *in_position = reinterpret_cast<float*>(input_positions.data + in_pt[0] * input_positions.stride);
			float *out_uv = reinterpret_cast<float*>(output_uv.data + i * output_uv.stride);

			out_uv[0] = in_position[0] * scale[0] + translation[0];
			out_uv[1] = in_position[1] * scale[1] + translation[1];
		}
		
		output_mesh.Release();
		input_mesh.Release();
		return kOfxStatOK;
	}
};

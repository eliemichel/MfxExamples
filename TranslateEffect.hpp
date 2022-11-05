#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include "utils.hpp"

/**
 * Translate the input geometry
 */
class TranslateEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "Translate";
	}

protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);

		// Add a vector3 parameter
		AddParam("translation", double3{ 0.0, 0.0, 0.0 })
			.Label("Translation") // Name used for display
			.Range(double3{ -10.0, -10.0, -10.0 }, double3{ 10.0, 10.0, 10.0 });

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle) override {
		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxAttributeProps input_positions;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(input_positions);

		float3 translation = to_float3(GetParam<double3>("translation").GetValue());

		MfxMeshProps mesh_props;
		input_mesh.FetchProperties(mesh_props);

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();

		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));

		if (mesh_props.constantFaceSize < 0) {
			output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
				.ForwardFrom(input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize));
		}

		output_mesh.Allocate(mesh_props);

		MfxAttributeProps output_positions;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(output_positions);

		// (NB: This can totally benefit from parallelization using e.g. OpenMP)
		for (int i = 0; i < mesh_props.pointCount; ++i) {
			float* in_p = input_positions.at<float>(i);
			float* out_p = output_positions.at<float>(i);
			out_p[0] = in_p[0] + translation[0];
			out_p[1] = in_p[1] + translation[1];
			out_p[2] = in_p[2] + translation[2];
		}

		output_mesh.Release();
		input_mesh.Release();

		return kOfxStatOK;
	}
};
#include <PluginSupport/MfxEffect>
#include "utils.hpp"

/**
 * Translate the input geometry
 */
class TranslateEffect : public MfxEffect {
	protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);
		
		// Add a vector3 parameter
		AddParam("translation", double3{0.0, 0.0, 0.0})
		.Label("Translation") // Name used for display
		.Range(double3{-10.0, -10.0, -10.0}, double3{10.0, 10.0, 10.0});
		
		return kOfxStatOK;
	}
	
	OfxStatus Cook(OfxMeshEffectHandle) override {
		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxAttributeProps input_positions;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(input_positions);

		double3 translation = GetParam<double3>("translation").GetValue();

		MfxMeshProps input_mesh_props;
		input_mesh.FetchProperties(input_mesh_props);
		int output_point_count = input_mesh_props.pointCount;
		int output_corner_count = input_mesh_props.cornerCount;
		int output_face_count = input_mesh_props.faceCount;
		
		// Extra properties related to memory usage optimization
		int output_no_loose_edge = input_mesh_props.noLooseEdge;
		int output_constant_face_count = input_mesh_props.constantFaceSize;

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();
		
		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));
		
		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
			.ForwardFrom(input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize));

		
		output_mesh.Allocate(
			output_point_count,
			output_corner_count,
			output_face_count,
			output_no_loose_edge,
			output_constant_face_count);

		MfxAttributeProps output_positions;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.FetchProperties(output_positions);
		
		// (NB: This can totally benefit from parallelization using e.g. OpenMP)
		float tx = static_cast<float>(translation[0]);
		float ty = static_cast<float>(translation[1]);
		float tz = static_cast<float>(translation[2]);
		for (int i = 0 ; i < output_point_count ; ++i) {
			float *in_p = attributeAt<float>(input_positions, i);
			float *out_p = attributeAt<float>(output_positions, i);
			out_p[0] = in_p[0] + tx;
			out_p[1] = in_p[1] + ty;
			out_p[2] = in_p[2] + tz;
		}

		output_mesh.Release();
		input_mesh.Release();

		return kOfxStatOK;

	}
	public:
	const char* GetName() override {
		return "Translate";
	}

};
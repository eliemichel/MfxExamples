#include <PluginSupport/MfxEffect>
#include <iostream>

/**
 * Copy geometry from the first extra input
 * (test case for extra inputs, this has no practical use)
 */
class CopyEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "Copy";
	}
protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput);

		AddInput("source")
		.Label("Source");
		
		AddInput(kOfxMeshMainOutput);

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle instance) override {
		MfxMesh source_mesh = GetInput("source").GetMesh();
		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();
		if (!source_mesh.IsValid()) {
			messageSuite->message(instance, "OfxMessageTypeError", NULL, "Invalid Source Input");
			return kOfxStatErrValue;
		}

		MfxMeshProps source_props;
		source_mesh.FetchProperties(source_props);

		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.ForwardFrom(source_mesh.GetPointAttribute(kOfxMeshAttribPointPosition));

		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(source_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));

		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
			.ForwardFrom(source_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize));

		output_mesh.Allocate(
			source_props.pointCount,
			source_props.cornerCount,
			source_props.faceCount,
			source_props.noLooseEdge,
			source_props.constantFaceSize);

		output_mesh.Release();
		source_mesh.Release();
		return kOfxStatOK;
	}
};

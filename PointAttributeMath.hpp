#include <PluginSupport/MfxEffect>

/**
 * For now, this is only doing an addition of two 1-dimensional float attribtues
 */
class PointAttributeMath : public MfxEffect {
public:
	const char* GetName() override {
		return "Point Attribute Math";
	}

protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput)
		.RequestPointAttribute("a", 1, MfxAttributeType::Float, MfxAttributeSemantic::None, false /* mandatory */)
		.RequestPointAttribute("b", 1, MfxAttributeType::Float, MfxAttributeSemantic::None, false /* mandatory */);

		AddInput(kOfxMeshMainOutput)
		.RequestPointAttribute("c", 1, MfxAttributeType::Float, MfxAttributeSemantic::None, false /* mandatory */);

		// !! It is not supposed to be allowed to call RequestPointAttribute on
		// the output, but we use it as proof of concept for a new method for
		// announcing in the description the output attributes.

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle instance) override {
		MfxMesh inputMesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMesh outputMesh = GetInput(kOfxMeshMainOutput).GetMesh();

		if (inputMesh.HasPointAttribute("a") && inputMesh.HasPointAttribute("b")) {
			outputMesh.AddPointAttribute("c", 1, MfxAttributeType::Float);
		} else {
			messageSuite->setPersistentMessage(instance, kOfxMessageError, nullptr, "Missing one of the input attributes!");
		}

#pragma region [Default geometry forwarding]
		MfxMeshProps inputProps;
		inputMesh.FetchProperties(inputProps);

		outputMesh.GetPointAttribute(kOfxMeshAttribPointPosition)
			.ForwardFrom(inputMesh.GetPointAttribute(kOfxMeshAttribPointPosition));

		outputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint)
			.ForwardFrom(inputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint));

		outputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize)
			.ForwardFrom(inputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize));

		outputMesh.Allocate(
			inputProps.pointCount,
			inputProps.cornerCount,
			inputProps.faceCount,
			inputProps.noLooseEdge,
			inputProps.constantFaceSize);
#pragma endregion [Default geometry forwarding]

		if (outputMesh.HasPointAttribute("c")) {
			MfxAttributeProps attribA, attribB, attribC;
			inputMesh.GetPointAttribute("a").FetchProperties(attribA);
			inputMesh.GetPointAttribute("b").FetchProperties(attribB);
			outputMesh.GetPointAttribute("c").FetchProperties(attribC);

			for (int i = 0 ; i < inputProps.pointCount ; ++i) {
				*attribC.at<float>(i) = *attribA.at<float>(i) + *attribB.at<float>(i);
			}
		}

		outputMesh.Release();
		inputMesh.Release();

		return kOfxStatOK;
	}
};

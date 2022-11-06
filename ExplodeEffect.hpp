#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include "utils.hpp"

#include <cmath>

/**
 * Copy each face (potentially several times) and translate it along its normal
 */
class ExplodeEffect : public MfxEffect {
public:
	const char* GetName() override {
		return "Explode";
	}
protected:
	OfxStatus Describe(OfxMeshEffectHandle) override {
		AddInput(kOfxMeshMainInput)
		.RequestCornerAttribute(
			"normal",
			3, MfxAttributeType::Float,
			MfxAttributeSemantic::Normal,
			false /* not mandatory */
		);
		
		AddInput(kOfxMeshMainOutput);

		// Number of duplicates of each faces
		AddParam("iterations", 3)
		.Label("Iterations")
		.Range(0, 20);
		
		// Translation added at each copy of a face
		AddParam("offset", 0.5)
		.Label("Offset")
		.Range(-10.0, 10.0);

		// Scale applied to each copy of a face
		AddParam("inset", 0.9)
		.Label("Inset Factor");

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle) override {
		MfxMesh input_mesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMeshProps input_props;
		input_mesh.FetchProperties(input_props);
		
		int iterations = GetParam<int>("iterations").GetValue();
		float offset = static_cast<float>(GetParam<double>("offset").GetValue());
		float inset = static_cast<float>(GetParam<double>("inset").GetValue());

		bool use_corner_normals = true;
		MfxAttributeProps input_normal;
		if (input_mesh.HasCornerAttribute("normal")) {
			input_mesh.GetCornerAttribute("normal").FetchProperties(input_normal);
			if (input_normal.componentCount < 3 || input_normal.type != MfxAttributeType::Float) {
				use_corner_normals = false; // incompatible type, so we deactivate this feature
			}
		} else {
			use_corner_normals = false; // no normals available, so we will compute them from faces
		}

		MfxMesh output_mesh = GetInput(kOfxMeshMainOutput).GetMesh();
		
		output_mesh.Allocate(
			input_props.pointCount + input_props.cornerCount * iterations,
			input_props.cornerCount * (iterations + 1),
			input_props.faceCount * (iterations + 1),
			input_props.noLooseEdge,
			input_props.constantFaceSize);

		MfxAttributeProps input_positions, input_corner_points, input_face_size;
		input_mesh.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(input_positions);
		input_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(input_corner_points);
		if (input_props.constantFaceSize == -1) {
			input_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(input_face_size);
		}
		
		MfxAttributeProps output_positions, output_corner_points, output_face_counts;
		output_mesh.GetPointAttribute(kOfxMeshAttribPointPosition).FetchProperties(output_positions);
		output_mesh.GetCornerAttribute(kOfxMeshAttribCornerPoint).FetchProperties(output_corner_points);
		output_mesh.GetFaceAttribute(kOfxMeshAttribFaceSize).FetchProperties(output_face_counts);

		// First copy the original points
		for (int i = 0; i < input_props.pointCount; ++i) {
			float* ipos = input_positions.at<float>(i);
			float* opos = output_positions.at<float>(i);
			opos[0] = ipos[0];
			opos[1] = ipos[1];
			opos[2] = ipos[2];
		}

		// Then add points at corners
		float totalInset = 1.0;
		for (int k = 0; k < iterations; ++k) {
			totalInset *= inset;
			int opointIndexOffset = input_props.pointCount + input_props.cornerCount * k;

			int i = 0;
			for (int f = 0; f < input_props.faceCount; ++f) {
				int cornerCount = (
					input_props.constantFaceSize >= 0
					? input_props.constantFaceSize
					: *input_face_size.at<int>(f)
				);

				// Compute barycenter and normal vector
				double3 faceNormal = double3{ 0.0, 0.0, 1.0 };
				double3 barycenter = double3{ 0.0, 0.0, 0.0 };
				double3 p0{};
				double3 p1{};
				for (int j = 0; j < cornerCount; ++j) {
					int* ipointIndex = input_corner_points.at<int>(i + j);
					float* ipos = input_positions.at<float>(*ipointIndex);

					barycenter[0] += ipos[0];
					barycenter[1] += ipos[1];
					barycenter[2] += ipos[2];

					if (j == 0) {
						p0[0] = ipos[0]; p0[1] = ipos[1]; p0[2] = ipos[2];
					}
					else if (j == 1) {
						p1[0] = ipos[0]; p1[1] = ipos[1]; p1[2] = ipos[2];
					}
					else if (j == 2) {
						double3 d0 = { p1[0] - p0[0],  p1[1] - p0[1], p1[2] - p0[2] };
						double3 d1 = { ipos[0] - p0[0], ipos[1] - p0[1], ipos[2] - p0[2] };
						faceNormal[0] = d0[1] * d1[2] - d0[2] * d1[1];
						faceNormal[1] = d0[2] * d1[0] - d0[0] * d1[2];
						faceNormal[2] = d0[0] * d1[1] - d0[1] * d1[0];
						double length = std::sqrt(faceNormal[0] * faceNormal[0] + faceNormal[1] * faceNormal[1] + faceNormal[2] * faceNormal[2]);
						faceNormal[0] /= length;
						faceNormal[1] /= length;
						faceNormal[2] /= length;
					}
				}
				barycenter[0] /= cornerCount;
				barycenter[1] /= cornerCount;
				barycenter[2] /= cornerCount;

				// iterate over the same corners, this time to create output points
				for (int j = 0; j < cornerCount; ++j, ++i) {
					int* ipointIndex = input_corner_points.at<int>(i);
					float* ipos = input_positions.at<float>(*ipointIndex);

					double3 normal = faceNormal;
					if (use_corner_normals) {
						float* cornerNormal = input_normal.at<float>(i);
						normal = double3{ cornerNormal[0], cornerNormal[1], cornerNormal[2] };
					}

					float* opos = output_positions.at<float>(opointIndexOffset + i);
					for (int c = 0; c < 3; ++c) {
						opos[c] = static_cast<float>(
							(ipos[c] - barycenter[c]) * totalInset
							+ barycenter[c]
							+ offset * (k + 1) * normal[c]
						);
					}
				}
			}
		}

		for (int i = 0; i < input_props.cornerCount; ++i) {
			int* ipointIndex = input_corner_points.at<int>(i);
			int* opointIndex = output_corner_points.at<int>(i);
			opointIndex[0] = ipointIndex[0];
		}
		for (int k = 0; k < iterations; ++k) {
			int pointIndexOffset = input_props.pointCount + input_props.cornerCount * k;
			int ocornerIndexOffset = (k + 1) * input_props.cornerCount;
			for (int i = 0; i < input_props.cornerCount; ++i) {
				int* opointIndex = output_corner_points.at<int>(ocornerIndexOffset + i);
				opointIndex[0] = pointIndexOffset + i;
			}
		}

		if (input_props.constantFaceSize < 0) {
			for (int k = 0; k < iterations + 1; ++k) {
				int ofaceIndexOffset = k * input_props.faceCount;
				for (int i = 0; i < input_props.faceCount; ++i) {
					int icount = (
						input_props.constantFaceSize >= 0
						? input_props.constantFaceSize
						: *input_face_size.at<int>(i)
					);
					int* ocount = output_face_counts.at<int>(ofaceIndexOffset + i);
					ocount[0] = icount;
				}
			}
		}
		
		output_mesh.Release();
		input_mesh.Release();
		return kOfxStatOK;

	}
};

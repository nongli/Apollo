#include "Triangle.h"
#include "Mesh.h"
#include "AABox.h"

namespace Apollo {

Triangle::Triangle(Mesh* m) : Primitive(m) {
    memset(&data, 0, sizeof(data));
    mesh = m;
}

Triangle::Triangle(Mesh* m, const Data& data) : Primitive(m) {
    this->data = data;
    mesh = m;
}

void Triangle::SetVertices(UINT32 v1, UINT32 v2, UINT32 v3) {
    data.A = v1;
    data.B = v2;
    data.C = v3;
}

void Triangle::SetNormals(UINT32 n1, UINT32 n2, UINT32 n3) {
    data.nA = n1;
    data.nB = n2;
    data.nC = n3;
}

void Triangle::SetUVs(UINT32 t1, UINT32 t2, UINT32 t3) {
    data.tA = t1;
    data.tB = t2;
    data.tC = t3;
}

void Triangle::Init() {
    Vector3 vA, vB, vC;
    vA	= mesh->m_vertices[data.A]; 
    vB	= mesh->m_vertices[data.B]; 
    vC	= mesh->m_vertices[data.C];

    // Compute geometric normal
    Vector3 n;
    Vector3 b = vC - vA;
    Vector3 c = vB - vA;
    n = c.Cross(b);

    k = n.MaxDimension();
    n	/=  n[k];
    n[k] = 1.0;

    UINT32 u = (k+1) % 3;
    UINT32 v = (k+2) % 3;

    const DOUBLE bx = vC[u] - vA[u];
    const DOUBLE by = vC[v] - vA[v];
    const DOUBLE cx = vB[u] - vA[u];
    const DOUBLE cy = vB[v] - vA[v];

    const DOUBLE bot = bx*cy - by*cx;

    n_u = n[u];
    n_v = n[v];
    n_d = vA.Dot(n);

    b_u = -by / bot;
    b_v = bx / bot;
    b_d = (by*vA[u] - bx*vA[v]) / bot;

    c_u = cy / bot;
    c_v = -cx / bot;
    c_d = (vA[v]*cx - vA[u]*cy) / bot;
}

Triangle::~Triangle() {
}

void Triangle::GetAABoundingBox(AABox& b) const {
    b.Add(mesh->m_vertices[data.A]);
    b.Add(mesh->m_vertices[data.B]);
    b.Add(mesh->m_vertices[data.C]);
}

void Triangle::Transform(const OrthonormalBasis& basis) {
    UNREFERENCED_PARAMETER(basis);
	ApolloException::NotYetImplemented();
}

FLOAT Triangle::SurfaceArea() const {
	ApolloException::NotYetImplemented();
    return 0;
}

void Triangle::GetNormalAndUV(SurfaceElement& surfel) const {
	DOUBLE u = surfel.intersection->u;
	DOUBLE v = surfel.intersection->v;

	Vector3 vNA = mesh->m_normals[data.nA];
	Vector3 vNB = mesh->m_normals[data.nB];
	Vector3 vNC = mesh->m_normals[data.nC];

	surfel.normal = (1-u-v)*vNA + u*vNB + v*vNC;
	surfel.normal.Normalize();

	if (mesh->m_uvs.size() == 0) {
		surfel.uv.u = surfel.uv.v = 0;
	}
	else {
		UV	uvA	= mesh->m_uvs[data.tA];
		UV	uvB	= mesh->m_uvs[data.tB];
		UV	uvC	= mesh->m_uvs[data.tC];
		surfel.uv.u = (FLOAT)((1-u-v)*uvA.u + u*uvB.u + v*uvC.u);
		surfel.uv.v = (FLOAT)((1-u-v)*uvA.v + u*uvB.v + v*uvC.v);
	}
}

}

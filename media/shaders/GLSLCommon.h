mat3 quatToMat3(vec4 q)
{
	mat3 result = mat3(1.0);
	
	float qxx = (q.x * q.x);
	float qyy = (q.y * q.y);
	float qzz = (q.z * q.z);
	float qxz = (q.x * q.z);
	float qxy = (q.x * q.y);
	float qyz = (q.y * q.z);
	float qwx = (q.w * q.x);
	float qwy = (q.w * q.y);
	float qwz = (q.w * q.z);

	result[0][0] = 1.0 - 2.0 * (qyy +  qzz);
	result[0][1] = 2.0 * (qxy + qwz);
	result[0][2] = 2.0 * (qxz - qwy);

	result[1][0] = 2.0 * (qxy - qwz);
	result[1][1] = 1.0 - 2.0 * (qxx +  qzz);
	result[1][2] = 2.0 * (qyz + qwx);

	result[2][0] = 2.0 * (qxz + qwy);
	result[2][1] = 2.0 * (qyz - qwx);
	result[2][2] = 1.0 - 2.0 * (qxx +  qyy);
	
	return result;
}

mat4 quatPosToMat4(vec4 q, vec3 pos)
{
	mat4 result;
	mat3 rotMtx = quatToMat3(q);
	result[0] = vec4(rotMtx[0], 0);
	result[1] = vec4(rotMtx[1], 0);
	result[2] = vec4(rotMtx[2], 0);
	result[3] = vec4(pos, 1);
	return result;
}
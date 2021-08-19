#include "InvisionHL.h"

void InvisionHL::SetProjectionViewMatrix(glm::mat4 view, glm::mat4 proj, glm::vec3 cameraPosition)
{
	gfxInstance.SetProjectionViewMatrix(view, proj, cameraPosition);
}
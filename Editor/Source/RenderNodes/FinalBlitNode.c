#include "FinalBlitNode.h"

#include <Euphorbe/Core/Log.h>
#include <Euphorbe/Graphics/Renderer.h>
#include <Euphorbe/Graphics/CommandBuffer.h>

typedef struct FinalBlitNodeData FinalBlitNodeData;
struct FinalBlitNodeData
{
	b32 first_render;
};

void FinalBlitNodeInit(E_RenderGraphNode* node, E_RenderGraphExecuteInfo* info)
{
	FinalBlitNodeData* data = node->node_data;
	data->first_render = 1;

	node->outputs[0] = E_MakeImage(info->width, info->height, E_ImageFormatRGBA16, E_ImageUsageSampled | E_ImageUsageTransferDest);
	node->output_count++;
}

void FinalBlitNodeClean(E_RenderGraphNode* node, E_RenderGraphExecuteInfo* info)
{
	FinalBlitNodeData* data = node->node_data;

	E_FreeImage(node->outputs[0]);
	free(data);
}

void FinalBlitNodeExecute(E_RenderGraphNode* node, E_RenderGraphExecuteInfo* info)
{
	FinalBlitNodeData* data = node->node_data;
	E_ImageLayout src_layout = data->first_render ? E_ImageLayoutUndefined : E_ImageLayoutShaderRead;

	E_CommandBuffer* cmd_buf = E_GetSwapchainCommandBuffer();

	E_CommandBufferImageTransitionLayout(cmd_buf, node->outputs[0],
		E_ImageAccessShaderRead, E_ImageAccessTransferWrite,
		src_layout, E_ImageLayoutTransferDest,
		E_ImagePipelineStageFragmentShader, E_ImagePipelineStageTransfer, 0);

	E_CommandBufferBlitImage(cmd_buf, E_GetRenderGraphNodeInputImage(&node->inputs[0]), node->outputs[0], E_ImageLayoutTransferSource, E_ImageLayoutTransferDest);

	E_CommandBufferImageTransitionLayout(cmd_buf, node->outputs[0],
		E_ImageAccessTransferWrite, E_ImageAccessShaderRead,
		E_ImageLayoutTransferDest, E_ImageLayoutShaderRead,
		E_ImagePipelineStageTransfer, E_ImagePipelineStageFragmentShader, 0);
}

void FinalBlitNodeResize(E_RenderGraphNode* node, E_RenderGraphExecuteInfo* info)
{
	FinalBlitNodeData* data = node->node_data;

	data->first_render = 1;
	E_FreeImage(node->outputs[0]);
	node->outputs[0] = E_MakeImage(info->width, info->height, E_ImageFormatRGBA16, E_ImageUsageSampled | E_ImageUsageTransferDest);
}

E_RenderGraphNode* CreateFinalBlitNode()
{
	E_RenderGraphNode* node = malloc(sizeof(E_RenderGraphNode));

	node->enabled = 1;
	node->init_func = FinalBlitNodeInit;
	node->clean_func = FinalBlitNodeClean;
	node->resize_func = FinalBlitNodeResize;
	node->execute_func = FinalBlitNodeExecute;
	node->name = "FinalBlitNode";
	node->node_data = malloc(sizeof(E_Image));

	node->input_count = 0;
	memset(node->inputs, 0, sizeof(node->inputs));

	return node;
}
#include "../nodeFactory.h"

#include "copyNode.h"
#include "fileInputNode.h"
#include "fileOutputNode.h"
#include "resizeNode.h"
#include "localDisplayNode.h"

// Register node types (actually definitions of static data members)
DEFINE_NFR(CopyNode, nt_copy);
DEFINE_NFR(FileInputNode, nt_file_input);
DEFINE_NFR(FileOutputNode, nt_file_output);
DEFINE_NFR(ResizeNode, nt_resize);
DEFINE_NFR(LocalDisplayNode, nt_local_display);


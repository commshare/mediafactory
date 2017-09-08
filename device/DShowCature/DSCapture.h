#ifndef _DS_CAPTURE_H_
#define _DS_CAPTURE_H_

#include "DSGraph.h"

class DSCapture
{
public:
    DSCapture();

    ~DSCapture();

    DSGraph * DShowGraph() { return ds_graph_; }

private:
    std::string CStringToString(const CString& mfcStr);

private:
    DSGraph*		ds_graph_;
};

#endif // _DS_CAPTURE_H_

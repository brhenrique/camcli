//
// Created by disc on 1/29/2021.
//

#ifndef CAM_CLI_MF_SESSION_H
#define CAM_CLI_MF_SESSION_H

/**
 * Initializes COM and Media Foundation. Shuts down both on destruction.
 */
class mf_session {
public:
    mf_session();

    ~mf_session();
};

#endif//CAM_CLI_MF_SESSION_H

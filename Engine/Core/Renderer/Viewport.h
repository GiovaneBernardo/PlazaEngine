#pragma once

namespace Plaza{
    struct PlViewport {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;

        template <class Archive> void serialize(Archive& archive) {
            archive(PL_SER(x), PL_SER(y), PL_SER(width), PL_SER(height), PL_SER(minDepth), PL_SER(maxDepth));
        }
    };
}

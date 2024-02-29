{
    global:
        CreateInterface;
        Plugify_*;
        extern "C++" {
            plugify::*;
        };
    local: 
        extern "C++" {
            std::*;
        };
};
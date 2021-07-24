#pragma once 



class exp_bad_param : public std::exception
{
public:
    const char* what()
    {
        return "param is empty or not enough!";
    }
    // ...  
};
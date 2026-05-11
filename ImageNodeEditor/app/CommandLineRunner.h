#pragma once

#include <QString>

class CommandLineRunner {
public:
    int run(const QString& workflowPath) const;
};

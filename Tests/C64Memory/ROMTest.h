#pragma once

#include <QObject>


class ROMTest : public QObject
{
    Q_OBJECT

public:
    explicit ROMTest();
    virtual ~ROMTest();

private slots:
    void testLoadData();
    void testLoadWrongSize();
    void testLoadWrongSizeKeepsData();

    void testLoadFile();
    void testLoadFileNotFound();
    void testLoadFileWrongSize();
};
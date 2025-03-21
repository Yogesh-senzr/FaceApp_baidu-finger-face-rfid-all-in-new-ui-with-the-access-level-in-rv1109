#ifndef FACEDB_H
#define FACEDB_H

#include <QObject>

class FaceDBPrivate;
class FaceDB : public QObject
{
    Q_OBJECT
public:
    explicit FaceDB(QObject *parent = nullptr);
    ~FaceDB();
public:
    static inline FaceDB *GetFaceDB(){static FaceDB g;return &g;}
//public:
//    bool selectRegisteredPerson(unsigned char * FaceFeature, const int &FaceFeatureSize);
//public:
//    int getFacedbRowCount();
//    int getIdentifyrecordMaxRid();
//public:

private:
    QScopedPointer<FaceDBPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(FaceDB)
    Q_DISABLE_COPY(FaceDB)
};

#endif // FACEDB_H

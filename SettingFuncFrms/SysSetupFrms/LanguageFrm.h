#ifndef LanguageFrm_H
#define LanguageFrm_H

#include <QDialog>


//语言设置
class LanguageFrmPrivate;
class LanguageFrm : public QDialog
{
    Q_OBJECT
public:
    explicit LanguageFrm(QWidget *parent = nullptr);
    ~LanguageFrm();
public:
    static inline LanguageFrm *GetInstance(){static LanguageFrm g;return &g;}    
    static LanguageFrm* GetLanguageInstance();
public://设置选中语言
    void setLanguageMode(const int &);
    int getLanguageMode()const;
    void UseLanguage(int forceload);//0: load, 1; 语言已更改,要 forceload
public:     
    signals:    void LanguageChaned();
 
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<LanguageFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif     
private:    
    static LanguageFrm* LangInst;
private:
    Q_DECLARE_PRIVATE(LanguageFrm)
    Q_DISABLE_COPY(LanguageFrm)
   


  



};

#endif // LanguageFrm_H

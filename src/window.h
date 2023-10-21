#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <gtkmm.h>
#include <gtkmm/application.h>
#include "visuals.h"

using namespace Glib;
using namespace Gtk;

class MyWindow : public ApplicationWindow {
protected:
    vector<string>                      filenames;
    int                                 filepos;
    corestring                          folder;
    corestring                          str;
public:
    /* constructor */                   MyWindow();
    /* destructor */                   ~MyWindow();
    ustring                             getFilenameDialog( const char *title, FileChooserAction fileAction, const char *extension );
    void                                on_load_clicked();
    void                                on_save_clicked();
    void                                on_menu_file_load();
    void                                on_menu_file_save();
    void                                ReadFile( ustring filename );
    void                                SaveFile( ustring filename );
    Box                                 gVBox, gHBox;
    MyArea                              drawArea;
};

class ExampleApplication : public Application {
protected:
    /* constructor */                   ExampleApplication( const char * appName );
    /* destructor */                   ~ExampleApplication();

public:
    static RefPtr<ExampleApplication>   create( const char * appName );

protected:
    void                                on_startup() override;
    void                                on_activate() override;

private:
    void                                on_window_hide();
    void                                on_menu_file_quit();
    void                                on_menu_help_about();

    RefPtr<Builder>                     m_refBuilder;
    RefPtr<MyWindow>                    myWindow;
};

#endif // WINDOW_H_INCLUDED
#include "window.h"

///////////////////////////////////////
MyWindow::MyWindow() : filepos( 0 ), gVBox( ORIENTATION_VERTICAL ), gHBox( ORIENTATION_HORIZONTAL ) {
///////////////////////////////////////
    auto buffer = getcwd( nullptr, 0 );
    folder = buffer;
    folder += "/Watchfaces/";
    free( buffer );

#ifdef linux
    mkdir( folder.c_str(), 0777 );
#else
    mkdir( folder.c_str() );
#endif

    DIR *dir = opendir( folder.c_str() );
    struct dirent *dp;

    while (( dp = readdir( dir )) != nullptr ) {
        string filename = dp->d_name;

        if( filename.substr( filename.find_last_of( "." ) + 1 ) == "bin" )
            filenames.push_back( filename );
    }

    closedir( dir );
    corestring file;
    if( filenames.size() ) {
        file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str());
        readFile( file.c_str() );
    }
    set_default_size( 1000, 800 );
    auto css_provider = CssProvider::create();
    css_provider->load_from_data( "* { background-image: none; background-color: #000000;}" );
    get_style_context()->add_provider( css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER );
    add( gVBox );
    gHBox.add( drawArea.gTypes );
    drawArea.gLoad.set_label( "load" );
    gHBox.add( drawArea.gLoad );
    drawArea.gSave.set_label( "save" );
    gHBox.add( drawArea.gSave );
    gHBox.add( drawArea.gNewTypes );
    drawArea.gAdd.set_label( "add" );
    gHBox.add( drawArea.gAdd );
    drawArea.gDel.set_label( "del" );
    gHBox.add( drawArea.gDel );
    drawArea.gLoad.signal_clicked().connect( sigc::mem_fun( this, &MyWindow::on_image_load_clicked ));
    drawArea.gSave.signal_clicked().connect( sigc::mem_fun( this, &MyWindow::on_image_save_clicked ));
    drawArea.gTypes.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_types_changed ));
    drawArea.gAdd.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_add_clicked ));
    drawArea.gDel.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_del_clicked ));

    gHBox.add( drawArea.gPosX );
    gHBox.add( drawArea.gPosY );
    gHBox.add( drawArea.gShift );
    drawArea.gPosX.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_width_changed ));
    drawArea.gPosY.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_height_changed ));

    drawArea.set_vexpand( true );
    gVBox.add( gHBox );
    drawArea.gHeightFrame.set_text( "0" );
    gHBox2.add( drawArea.gHeightFrame );
    drawArea.gAddHeight.set_label( "add height" );
    gHBox2.add( drawArea.gAddHeight );
    gHBox2.add( drawArea.gDefvalue );
    drawArea.gAddHeight.signal_clicked().connect( sigc::mem_fun( drawArea, &MyArea::on_add_height_clicked ));
    drawArea.gDefvalue.signal_changed().connect( sigc::mem_fun( drawArea, &MyArea::on_def_value_changed ));
    gVBox.add( gHBox2 );
    gVBox.add( drawArea );
    signal_timeout().connect( sigc::mem_fun( drawArea, &MyArea::on_timeout), 20 );
    drawArea.signal_draw().connect( sigc::mem_fun( drawArea, &MyArea::on_draw ));
    show_all_children();
    signal_motion_notify_event().connect([&]( GdkEventMotion* event )->bool {
        mousePosition mp;
        mp.x = event->x;
        mp.y = event->y;
        drawArea.on_mouse_moved( mp );
        return false;
    }, false );
    signal_button_press_event().connect([&]( GdkEventButton* event )->bool {
        mousePosition mp;
        mp.x = event->x;
        mp.y = event->y;
        drawArea.on_mouse_pressed( event->button, mp );
        return false;
    }, false );
    signal_button_release_event().connect([&]( GdkEventButton* event )->bool {
        mousePosition mp;
        mp.x = event->x;
        mp.y = event->y;
        drawArea.on_mouse_released( event->button, mp );
        return false;
    }, false );
    signal_key_press_event().connect([&]( GdkEventKey* event )->bool {
        if( GDK_KEY_Escape == event->keyval )
            close();
        if( filenames.size() ) {
            if( GDK_KEY_q == event->keyval ) {
                if( --filepos < 0 )
                    filepos = filenames.size() - 1;

                if(( int ) filenames.size() > filepos ) {
                    corestring file;
                    file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str());
                    readFile( file.c_str() );
                    drawArea.resetShift();
                }
            }
            if( GDK_KEY_w == event->keyval ) {
                if( ++filepos >= ( int ) filenames.size() )
                    filepos = 0;

                if( ( int ) filenames.size() > filepos ) {
                    corestring file;
                    file.format( "%s%s", folder.c_str(), filenames[ filepos ].c_str() );
                    readFile( file.c_str() );
                    drawArea.resetShift();
                }
            }
        }
        if( GDK_KEY_c == event->keyval ) {
            drawArea.createPreview();
        }
        if( GDK_KEY_d == event->keyval ) {
            drawArea.debug = !drawArea.debug;
        }
        if( GDK_KEY_F1 == event->keyval ) {
            drawArea.preview = true;
            drawArea.resetShift();
            drawArea.gTypes.set_active_text("");
            drawArea.gNewTypes.set_active_text("");
        }
        if( GDK_KEY_F2 == event->keyval ) {
            drawArea.preview = false;
            drawArea.resetShift();
            drawArea.gTypes.set_active_text("");
            drawArea.gNewTypes.set_active_text("");
        }
        if(( GDK_KEY_0 > event->keyval || GDK_KEY_9 < event->keyval ) && GDK_KEY_Left != event->keyval && GDK_KEY_Right != event->keyval &&
             GDK_KEY_BackSpace != event->keyval && GDK_KEY_Delete != event->keyval && GDK_KEY_End != event->keyval && GDK_KEY_Home != event->keyval &&
             ( get_focus() != &drawArea.gHeightFrame || GDK_KEY_minus != event->keyval )) {
            if( get_focus() == &drawArea.gPosX || get_focus() == &drawArea.gPosY || get_focus() == &drawArea.gHeightFrame ) {
                return true;
            }
        }
        return false;
    }, false);
};

///////////////////////////////////////
MyWindow::~MyWindow() {};
///////////////////////////////////////

///////////////////////////////////////
ustring MyWindow::getFilenameDialog( const char *title, FileChooserAction fileAction, const char *extension ) {
///////////////////////////////////////
    auto dialog = new FileChooserDialog( title, fileAction );
    dialog->set_transient_for( *this );
    dialog->set_modal( true );
    dialog->set_current_folder( folder.c_str() );

    dialog->add_button("Cancel", RESPONSE_CANCEL);
    if( FILE_CHOOSER_ACTION_SAVE == fileAction )
        dialog->add_button("Save", RESPONSE_OK);
    if( FILE_CHOOSER_ACTION_OPEN == fileAction )
        dialog->add_button("Load", RESPONSE_OK);

    auto filter_bin = FileFilter::create();
    str.format( "%s files", extension );
    filter_bin->set_name( str.c_str() );
    str.format( "*.%s", extension );
    filter_bin->add_pattern( str.c_str() );
    dialog->add_filter( filter_bin );

    auto filter_any = FileFilter::create();
    filter_any->set_name( "Any files" );
    filter_any->add_pattern( "*" );
    dialog->add_filter( filter_any );

    int result = dialog->run();

    corestring filename;

    switch( result ) {
        case RESPONSE_OK: {
            filename = dialog->get_filename();
            break;
        }
        case RESPONSE_CANCEL: {
            break;
        }
        default: {
            break;
        }
    }

    if( FILE_CHOOSER_ACTION_SAVE == fileAction && extension != filename.substr( filename.find_last_of( "." ) + 1 )) {
        str.format( ".%s", extension );
        filename.append( str );
    }

    dialog->close();

    if( RESPONSE_OK == result ) {
        return filename;
    }
    return "";
}

///////////////////////////////////////
void MyWindow::on_image_load_clicked() {
///////////////////////////////////////
    if( !drawArea.gTypes.get_active_text().size() )
        return;

    int itemid = drawArea.itemTextToID( drawArea.gTypes.get_active_text().c_str() );

    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_OPEN, "png" );

    if( filename.size() ) {
        auto image = Gdk::Pixbuf::create_from_file( filename.c_str() );
        auto &destination = drawArea.binfile.items[ itemid ];
        destination.width = image->get_width();
        destination.height = image->get_height() / destination.imgcount;
        destination.count = destination.width * destination.height * destination.imgcount;
        destination.RGB32 = shared_ptr<unsigned int[]>( new unsigned int[ destination.count ]);
        if( image->get_has_alpha() ) {
            memcpy( destination.RGB32.get(), image->get_pixels(), destination.count * 4 );
        } else {
            unsigned char *rgbCols = ( unsigned char * ) image->get_pixels();
            int rowstride = image->get_rowstride();
            unsigned char *destCols = ( unsigned char * ) destination.RGB32.get();
            for( size_t y = 0; y < destination.height * destination.imgcount; ++y ) {
                for( size_t x = 0; x < destination.width; ++x ) {
                    rgbColor &rgbs = *( rgbColor * ) &rgbCols[ rowstride * y + x * sizeof( rgbColor )];
                    auto cols = &destCols[( destination.width * y + x ) * 4 ];
                    cols[ 0 ] = rgbs.r;
                    cols[ 1 ] = rgbs.g;
                    cols[ 2 ] = rgbs.b;
                    cols[ 3 ] = 0xff;
                }
            }
        }
        destination.toOrig();
        destination.toRGB32();
    }
};

///////////////////////////////////////
void MyWindow::on_image_save_clicked() {
///////////////////////////////////////
    if( !drawArea.gTypes.get_active_text().size() )
        return;

    int itemid = drawArea.itemTextToID( drawArea.gTypes.get_active_text().c_str() );

    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_SAVE, "png" );

    if( filename.size() ) {
        auto &item = drawArea.binfile.items[ itemid ];
        drawArea.view.getFromMemory(( unsigned char * ) item.RGB32.get(), item.width, item.height * item.imgcount );
        drawArea.view.img->save( filename, "png" );
    }
};

///////////////////////////////////////
void MyWindow::on_bin_file_load() {
///////////////////////////////////////
    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_OPEN, "bin" );

    if( filename.size() ) {
        readFile( filename );
    }
}

///////////////////////////////////////
void MyWindow::on_bin_file_save() {
///////////////////////////////////////
    ustring filename = getFilenameDialog( "Please choose a file", FILE_CHOOSER_ACTION_SAVE, "bin" );

    if( filename.size() ) {
        saveFile( filename );
    }
}

///////////////////////////////////////
void MyWindow::readFile( ustring filename ) {
///////////////////////////////////////
    drawArea.setup( filename.c_str() );
    corestring title;
    title.format( "MK66 Watchface Editor - %s", filename.c_str() );
    set_title( title.c_str() );
}

///////////////////////////////////////
void MyWindow::saveFile( ustring filename ) {
///////////////////////////////////////
    drawArea.write( filename.c_str() );
}

///////////////////////////////////////
ExampleApplication::ExampleApplication( const char * appName ) : Application( appName ) {
///////////////////////////////////////
    set_application_name( appName );
    setlocale( LC_ALL, "C" );
}

///////////////////////////////////////
ExampleApplication::~ExampleApplication() {
///////////////////////////////////////
    on_window_hide();
}

///////////////////////////////////////
RefPtr<ExampleApplication> ExampleApplication::create( const char * appName ) {
///////////////////////////////////////
    return RefPtr<ExampleApplication>( new ExampleApplication( appName ));
}

///////////////////////////////////////
void ExampleApplication::on_startup() {
///////////////////////////////////////
    Application::on_startup();
}

///////////////////////////////////////
void ExampleApplication::on_activate() {
///////////////////////////////////////
    myWindow = RefPtr<MyWindow>( new MyWindow() );

    add_window( *myWindow.get() );

    myWindow->signal_hide().connect( sigc::mem_fun( *this, &ExampleApplication::on_window_hide ));
    add_action( "load", sigc::mem_fun( myWindow.get(), &MyWindow::on_bin_file_load ));
    add_action( "save", sigc::mem_fun( myWindow.get(), &MyWindow::on_bin_file_save ));
    add_action( "quit", sigc::mem_fun( *this, &ExampleApplication::on_menu_file_quit ));
    add_action( "about", sigc::mem_fun( *this, &ExampleApplication::on_menu_help_about ));

    refBuilder = Builder::create();

    ustring ui_info =
        "<interface>"
        "  <!-- menubar -->"
        "  <menu id='menu-example'>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_File</attribute>"
        "      <section>"
        "        <item>"
        "          <attribute name='label' translatable='yes'>Load bin</attribute>"
        "          <attribute name='action'>app.load</attribute>"
        "          <attribute name='accel'>&lt;Primary&gt;n</attribute>"
        "        </item>"
        "        <item>"
        "          <attribute name='label' translatable='yes'>Save bin</attribute>"
        "          <attribute name='action'>app.save</attribute>"
        "          <attribute name='accel'>&lt;Primary&gt;n</attribute>"
        "        </item>"
        "      </section>"
        "      <section>"
        "        <item>"
        "          <attribute name='label' translatable='yes'>_Quit</attribute>"
        "          <attribute name='action'>app.quit</attribute>"
        "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
        "        </item>"
        "      </section>"
        "    </submenu>"
        "    <submenu>"
        "      <attribute name='label' translatable='yes'>_Help</attribute>"
        "      <section>"
        "        <item>"
        "          <attribute name='label' translatable='yes'>_About</attribute>"
        "          <attribute name='action'>app.about</attribute>"
        "        </item>"
        "      </section>"
        "    </submenu>"
        "  </menu>"
        "</interface>";

    refBuilder->add_from_string( ui_info );

    auto object = refBuilder->get_object( "menu-example" );
    auto gmenu = RefPtr<Gio::Menu>::cast_dynamic( object );

    if ( !gmenu ) {
        g_warning( "GMenu not found" );
    } else {
        set_menubar( gmenu );
    }
    myWindow->show_all();
}

///////////////////////////////////////
void ExampleApplication::on_window_hide() {
///////////////////////////////////////
    auto window = myWindow.release(); ( void ) window;
    auto builder = refBuilder.release(); ( void ) builder;
}

///////////////////////////////////////
void ExampleApplication::on_menu_file_quit() {
///////////////////////////////////////
    vector<Window*> windows = get_windows();

    if ( windows.size() > 0 )
        windows[0]->hide();
}

///////////////////////////////////////
void ExampleApplication::on_menu_help_about() {
///////////////////////////////////////
    MessageDialog messageBox( *myWindow.get(), "MK66 Watchface editor!\nCreated by János Klingl", true, MESSAGE_INFO, BUTTONS_OK, true );
    messageBox.set_title( "About" );
    messageBox.set_modal();
    messageBox.set_position( WindowPosition::WIN_POS_CENTER );
    messageBox.run();
}

///////////////////////////////////////
int main(int argc, char *argv[]) {
///////////////////////////////////////
    auto app = ExampleApplication::create( "mk66.watchface.editor.app" );

    return app->run( argc, argv );

    return 0;
}

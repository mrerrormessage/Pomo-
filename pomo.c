
//code by Robert Grider 
//Mar 2, 2010

#include <gtk/gtk.h>
#include <stdlib.h>

#define NUMMINS 25

int numsecs;
guint tickid;
GtkTextBuffer * buf;
struct pomo_prog  pp;

typedef enum{
  BEGIN,
  POMODORO,
  POMO_TO_BREAK,
  BREAK,
  NUM_STATES}
  Prog_State;

struct pomo_prog {
  Prog_State state;
  int pomo_length; //pomodoro length, in minutes
  int break_length; //break length, in minutes
  int seconds_remaining; //seconds remaining in the current pomodoro
  GtkTextBuffer * texbuf; //holds the text buffer that is rendered into
  GtkWindow * win; //holds the window
  GtkButton * button; //holds the button
};

//initializes the incoming program
void init_pomo_prog(struct pomo_prog * p,
		    GtkTextBuffer * t,
		    GtkWindow * w,
		    GtkButton * b){
  p->pomo_length = 25;
  p->break_length = 5;
  p->texbuf = t;
  p->win = w;
  p->button = b;
  p->seconds_remaining = 0;
  p->state = BEGIN;
  return;
}

void getClockString( int numsecs, char * strret){

  //get remaining number of seconds
  int displaysecs = numsecs % 60;
  int displaymins = numsecs / 60;

  //flush to buffer
  sprintf(strret, "%i:%i", displaymins, displaysecs);

  if(displaysecs < 10){
    sprintf(strret, "%i:0%i", displaymins, displaysecs);
  }


  return;
}

gboolean tick (gpointer data){
  struct pomo_prog * p = (struct pomo_prog *) data;

  p->seconds_remaining = p->seconds_remaining >= 1 ? (p->seconds_remaining - 1) : 0;  
  char dispstr[128];
  char windowstr[128];
  char buttonstr[128];
  switch (p->state)
    {
    case BEGIN:
      sprintf(windowstr, "Pomodoro!");
      sprintf(dispstr, "Ready?");
      sprintf(buttonstr, "Go!");
      break;
    case POMODORO:
      getClockString( p->seconds_remaining, dispstr);
      sprintf(windowstr, "Pomodoro! - %s", dispstr);
      sprintf(buttonstr, "Squash?");
      if( 0 == p->seconds_remaining ){
	p->state = POMO_TO_BREAK;
      }
      break;
    case POMO_TO_BREAK:
      sprintf(windowstr, "Break time!");
      sprintf(dispstr, "Break!");
      sprintf(buttonstr, "Begin");
      break;
    case BREAK:
      getClockString( p->seconds_remaining, dispstr);
      sprintf(windowstr, "Break! - %s", dispstr);
      sprintf(buttonstr, "Enjoy your break!");
      if( 0 == p->seconds_remaining ){
	gtk_widget_set_sensitive( GTK_WIDGET(p->button), TRUE );
	p->state = BEGIN;
      }
      break;
    default:
      break;
    }


  gtk_text_buffer_set_text(buf, dispstr, -1);
  
  gtk_window_set_title( p->win, windowstr);
  
  gtk_button_set_label( p->button, buttonstr);

  return TRUE;

}

void click_event(GtkWidget * g, gpointer data ){
  struct pomo_prog *p = (struct pomo_prog *)data;
  switch( p->state) 
    {
    case BEGIN:
      p->state = POMODORO;
      p->seconds_remaining = 10; //p->pomo_length * 60 + 1;
      break;
    case POMODORO:
      //when it's a Pomodoro, the button with say "squash?", so we transition back to begin
      p->state = BEGIN;
      p->seconds_remaining = 0;
      break;
    case BREAK:
      //don't need to worry about the button when we're on break, it will be set as inactive
      break;
    case POMO_TO_BREAK:
      p->state = BREAK;
      p->seconds_remaining = 5;// p->break_length * 60 + 1;
      gtk_widget_set_sensitive( GTK_WIDGET( p->button ), FALSE);
      break;

    default:
      break; 
    }

}

int main ( int argc, char ** argv ) {

  GtkBuilder *builder;
  GtkWidget * window;
  GtkWidget * button;
  GtkWidget * textview;
  GtkTextView * mytextview;
  guint clickint;
  GError * error = NULL;

  //pp = malloc(sizeof(struct pomo_prog));
 

  //initialize
  gtk_init(&argc, &argv);

  //make the builder
  builder = gtk_builder_new();

  //load the pomo UI
  if(! gtk_builder_add_from_file( builder, "pomo.glade", &error) ){
    g_warning( "%s", error->message);
    g_free( error );
    return( 1 );
  }

  //get the window object
  window = GTK_WIDGET ( gtk_builder_get_object(builder, "window1") );

  //get the button object
  button = GTK_WIDGET ( gtk_builder_get_object(builder, "button1") );

  //get the textview object
  textview = GTK_WIDGET (gtk_builder_get_object(builder, "textview1") );
  mytextview = GTK_TEXT_VIEW( textview);

  //set buffer
  buf = gtk_text_buffer_new(NULL);
  gtk_text_view_set_buffer(mytextview, buf);


  //set the font size of the textview to be something larger than 12 point Times NewRoman
  PangoFontDescription * fnt = pango_font_description_from_string( "Serif 36");
  gtk_widget_modify_font( textview, fnt );
  pango_font_description_free(fnt); 
  
  //get the program state data structure filled out
  init_pomo_prog(&pp, buf, GTK_WINDOW( window ), GTK_BUTTON( button ) );
  printf("length: %i", pp.pomo_length);

  //conect signals

  //timeout
  tickid = g_timeout_add( 1000, tick , &pp);

  //click signal
  clickint = g_signal_connect( G_OBJECT(button) , "clicked", G_CALLBACK( click_event),(gpointer) &pp); 

  
  //miscellaneous
  gtk_builder_connect_signals( builder, NULL );

  

  //destroy builder
  g_object_unref( G_OBJECT( builder ) );
  
  //show and run
  gtk_widget_show( window );
  gtk_main();

  //free(pp);
  return( 0 );

}

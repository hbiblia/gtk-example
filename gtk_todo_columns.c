// gcc gtk_todo_columns.c -o gtk_todo_columns `pkg-config --libs --cflags gtk4`
// Author: slam - hbiblia@gmail.com
// OS: Windows

#include <stdio.h>
#include <gtk/gtk.h>

static GListStore* store;
static guint unique_id = 0;
static GtkSingleSelection* selection;

// -------------------------
// BEGIN DEFINE TYPE
// ------------------------
typedef struct _TodoItem TodoItem;

#define TODO_TYPE_ITEM (todo_item_get_type())
G_DECLARE_FINAL_TYPE(TodoItem, todo_item, TODO, ITEM, GObject);

struct _TodoItem
{
  GObject parent;

  gchar* name;
  bool completed;
  guint id;
  GListStore* children;
  GListStore* root; // parent
};

G_DEFINE_TYPE(TodoItem, todo_item, G_TYPE_OBJECT);
static void todo_item_init(TodoItem* item) {}
static void todo_item_class_init(TodoItemClass* kclass) {}

TodoItem* todo_item_new(const gchar* name)
{
  TodoItem* item = g_object_new(TODO_TYPE_ITEM, NULL);
  item->name = g_strdup(name);
  item->completed = FALSE;
  item->id = unique_id;
  item->children = g_list_store_new(TODO_TYPE_ITEM);
  item->root = NULL;
  unique_id++;
  return item;
}
// -------------------------
// END DEFINE TYPE
// -------------------------

static TodoItem* fn_get_selected(GtkSingleSelection* selection)
{
  GtkTreeListRow* row_item = gtk_single_selection_get_selected_item(selection);
  TodoItem* item = gtk_tree_list_row_get_item(row_item);
  return item;
}

// Creamos un nuevo item en la lista
static void signal_create_item(GtkWidget* button, GtkEntry* entry)
{
  GtkEntryBuffer* buffer = GTK_ENTRY_BUFFER(gtk_entry_get_buffer(GTK_ENTRY(entry)));
  const gchar* text = gtk_entry_buffer_get_text(buffer);

  if (strlen(text) <= 0) {
    g_warning("No contiene texto para agregar a lista.");
    return;
  }

  TodoItem* item = todo_item_new(text);

  // Tenemos un item seleccionado ??
  guint post = gtk_single_selection_get_selected(selection);
  if (post != GTK_INVALID_LIST_POSITION)
  {
    TodoItem* parent = fn_get_selected(selection);
    item->root = parent->children;
    g_list_store_append(parent->children, item);
  }
  else {
    item->root = store;
    g_list_store_append(store, item);
  }

  gtk_entry_buffer_delete_text(buffer, 0, strlen(text));
}

// Deseleccionamos un item
static void signal_unselect_item(GtkWidget* button, GtkSingleSelection* selection)
{
  gtk_single_selection_set_selected(selection, -1);
}

// Borramos un item seleccionado.
static void signal_remove_item(GtkWidget* button, GtkSingleSelection* selection)
{
  guint post = gtk_single_selection_get_selected(selection);
  if (post != GTK_INVALID_LIST_POSITION)
  {
    TodoItem* item = fn_get_selected(selection);
    if (g_list_store_find(item->root, item, &post)) {
      g_list_store_remove(item->root, post);
    }
  }
  else g_warning("Debe seleccionar un item de la lista.");
}

// --------------------------------------
// BEGIN::Column1: COMPLETED
// --------------------------------------
static void factory_setup_completed(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkWidget* checkbox, * expander;

  expander = gtk_tree_expander_new();
  gtk_list_item_set_child(GTK_LIST_ITEM(list_item), expander);

  checkbox = gtk_check_button_new();
  gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), checkbox);
}

static void factory_bind_completed(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkWidget* expander, * child;
  GtkTreeListRow* row_item;

  row_item = gtk_list_item_get_item(list_item);
  TodoItem* item = gtk_tree_list_row_get_item(row_item);

  expander = gtk_list_item_get_child(list_item);
  gtk_tree_expander_set_list_row(GTK_TREE_EXPANDER(expander), row_item);

  child = gtk_tree_expander_get_child(GTK_TREE_EXPANDER(expander));
  gtk_check_button_set_active(GTK_CHECK_BUTTON(child), item->completed);
}
// --------------------------------------
// END::Column1: COMPLETED
// --------------------------------------

// --------------------------------------
// BEGIN::Column2: NAME
// --------------------------------------
static void factory_setup_name(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkWidget* label;

  label = gtk_label_new("");
  gtk_list_item_set_child(GTK_LIST_ITEM(list_item), label);
  gtk_label_set_xalign(GTK_LABEL(label), 0);
  gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
}

static void factory_bind_name(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkWidget* expander, * child;
  GtkTreeListRow* row_item;

  row_item = gtk_list_item_get_item(list_item);
  TodoItem* item = gtk_tree_list_row_get_item(row_item);

  GtkWidget* label = gtk_list_item_get_child(GTK_LIST_ITEM(list_item));
  gtk_label_set_label(GTK_LABEL(label), item->name);
}
// --------------------------------------
// END::Column2: NAME
// --------------------------------------

// --------------------------------------
// BEGIN::Column3: ID
// --------------------------------------
static void factory_setup_id(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkWidget* label = gtk_label_new("");
  gtk_list_item_set_child(GTK_LIST_ITEM(list_item), label);
}

static void factory_bind_id(GtkSignalListItemFactory* factory, GtkListItem* list_item, gpointer data)
{
  GtkTreeListRow* row_item = gtk_list_item_get_item(GTK_LIST_ITEM(list_item));
  TodoItem* item = gtk_tree_list_row_get_item(row_item);

  GtkWidget* label = gtk_list_item_get_child(GTK_LIST_ITEM(list_item));
  gtk_label_set_label(GTK_LABEL(label), g_strdup_printf("%d", item->id));
}
// --------------------------------------
// END::Column3: ID
// --------------------------------------

static GListModel* fn_model_create(GObject* object, gpointer data)
{
  TodoItem* item = TODO_ITEM(object);

  if (item->children) {
    // if (g_list_model_get_n_items(G_LIST_MODEL(item->children)) != 0)
    return G_LIST_MODEL(g_object_ref(item->children));
  }

  return NULL;
}

static void activate(GtkApplication* app, gpointer user_data)
{
  GtkWidget* vbox, * vbox_entry, * colview, * scroll, * entry, * btn_remove, * btn_unseleted, * btn_add;
  GtkTreeListModel* model;
  // GtkSingleSelection* selection;
  GtkListItemFactory* factory;
  GtkColumnViewColumn* column;

  GtkWidget* window = gtk_application_window_new(app);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
  gtk_window_set_title(GTK_WINDOW(window), "gtk_todo_columns");

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(window), vbox);
  {
    vbox_entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_margin_start(vbox_entry, 5);
    gtk_widget_set_margin_end(vbox_entry, 5);
    gtk_widget_set_margin_top(vbox_entry, 5);
    gtk_widget_set_margin_bottom(vbox_entry, 5);
    gtk_box_append(GTK_BOX(vbox), vbox_entry);
    {

      btn_remove = gtk_button_new_from_icon_name("user-trash-symbolic");
      gtk_box_append(GTK_BOX(vbox_entry), btn_remove);

      entry = gtk_entry_new();
      gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Escribe");
      gtk_widget_set_hexpand(entry, TRUE);
      gtk_box_append(GTK_BOX(vbox_entry), entry);

      btn_add = gtk_button_new_from_icon_name("list-add-symbolic");
      gtk_box_append(GTK_BOX(vbox_entry), btn_add);

      btn_unseleted = gtk_button_new_from_icon_name("image-missing-symbolic");
      gtk_box_append(GTK_BOX(vbox_entry), btn_unseleted);
    }

    scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(vbox), scroll);
    {
      store = g_list_store_new(TODO_TYPE_ITEM);
      model = gtk_tree_list_model_new(G_LIST_MODEL(store), FALSE, TRUE, (GtkTreeListModelCreateModelFunc)fn_model_create, NULL, NULL);
      selection = gtk_single_selection_new(G_LIST_MODEL(model));
      gtk_single_selection_set_autoselect(selection, FALSE);
      gtk_single_selection_set_can_unselect(selection, TRUE);

      colview = gtk_column_view_new(GTK_SELECTION_MODEL(selection));
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), colview);

      // Column 1:COMPLETED
      factory = gtk_signal_list_item_factory_new();
      g_signal_connect(factory, "setup", G_CALLBACK(factory_setup_completed), NULL);
      g_signal_connect(factory, "bind", G_CALLBACK(factory_bind_completed), NULL);
      {
        column = gtk_column_view_column_new("Completed", factory);
        gtk_column_view_column_set_expand(column, FALSE);
        gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), column);
        g_object_unref(column);
      }

      // Column 2:NAME
      factory = gtk_signal_list_item_factory_new();
      g_signal_connect(factory, "setup", G_CALLBACK(factory_setup_name), NULL);
      g_signal_connect(factory, "bind", G_CALLBACK(factory_bind_name), NULL);
      {
        column = gtk_column_view_column_new("Name", factory);
        gtk_column_view_column_set_expand(column, TRUE);
        gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), column);
        g_object_unref(column);
      }

      // Column 3:ID
      factory = gtk_signal_list_item_factory_new();
      g_signal_connect(factory, "setup", G_CALLBACK(factory_setup_id), NULL);
      g_signal_connect(factory, "bind", G_CALLBACK(factory_bind_id), NULL);
      {
        column = gtk_column_view_column_new("Id", factory);
        gtk_column_view_column_set_visible(column, FALSE);
        gtk_column_view_column_set_expand(column, FALSE);
        gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), column);
        g_object_unref(column);
      }
    }
  }

  g_signal_connect(btn_remove, "clicked", G_CALLBACK(signal_remove_item), selection);
  g_signal_connect(btn_add, "clicked", G_CALLBACK(signal_create_item), entry);
  g_signal_connect(btn_unseleted, "clicked", G_CALLBACK(signal_unselect_item), selection);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv)
{
  GtkApplication* app;
  int status;

  app = gtk_application_new("com.todoList1.gtk", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
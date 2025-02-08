#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <gtkmm.h>

namespace MieruCache {
struct CacheEvent {
    unsigned int hart_id;
    unsigned int initiator_id;
    unsigned int index;
    std::string address;
    unsigned char old_state;
    unsigned char new_state;
};

class MainWindow : public Gtk::Window {
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView tree_view;
    Gtk::TreeModelColumn<int> index;
    std::vector<Gtk::TreeModelColumn<std::string>> cache_lines;
    Gtk::TreeModel::ColumnRecord record;
    Glib::RefPtr<Gtk::ListStore> list_store;
    Gtk::Frame caches_frame;
    Gtk::Scale time_slider;
    Gtk::ToolButton prev_button, next_button;
    Gtk::SeparatorToolItem separator;
    Gtk::Toolbar toolbar;
    Gtk::Box box, toolbar_box;
public:
    MainWindow(int width, int height, int num_harts, int num_entries, int num_ways, int num_events) {
        set_default_size(width, height);
        set_title("MieruCache");

        // Set up toolbar
        prev_button.set_icon_name("go-previous");
        toolbar.append(prev_button);
        next_button.set_icon_name("go-next");
        toolbar.append(next_button);
        toolbar.append(separator);

        // Set up time slider
        time_slider.set_range(0, num_events - 1);
        time_slider.set_value(0);
        time_slider.set_digits(0);
        time_slider.set_increments(1, 1);
        time_slider.set_draw_value(true);
        time_slider.set_value_pos(Gtk::POS_LEFT);
        // TODO: Show the time in the slider instead of the index of the vector

        toolbar_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        toolbar_box.pack_start(toolbar, Gtk::PACK_SHRINK);
        toolbar_box.pack_start(time_slider, Gtk::PACK_EXPAND_WIDGET);

        // Register columns
        record.add(index);
        for (int i = 0; i < num_harts; i++) {
            for (int j = 0; j < num_ways; j++) {
                record.add(cache_lines.emplace_back());
            }
        }
        // Create list store
        list_store = Gtk::ListStore::create(record);
        // Set list store
        tree_view.set_model(list_store);
        // Append columns to the tree view
        tree_view.append_column("Index", index);
        for (int i = 0; i < num_harts; i++) {
            for (int j = 0; j < num_ways; j++) {
                tree_view.append_column("Hart[" + std::to_string(i) + "], Way[" + std::to_string(j) + "]", cache_lines[i * num_ways + j]);
            }
        }
        // Add rows
        for (int i = 0; i < num_entries; i++) {
            auto row = *(list_store->append());
            row[index] = i;
        }
        // Add tree view to scrolled window
        scrolled_window.add(tree_view);

        // Add scrolled window to frame
        scrolled_window.set_border_width(2);
        caches_frame.add(scrolled_window);
        caches_frame.set_label("Caches");
        set_border_width(8);

        // Put everything in a box
        box.set_orientation(Gtk::ORIENTATION_VERTICAL);
        // box.pack_start(toolbar, Gtk::PACK_SHRINK);
        // box.pack_start(time_slider, Gtk::PACK_SHRINK);
        box.pack_start(toolbar_box, Gtk::PACK_SHRINK);
        box.pack_start(caches_frame, Gtk::PACK_EXPAND_WIDGET);
        add(box);

        show_all_children();
    }
    virtual ~MainWindow() = default;
};
}

static std::vector<std::pair<unsigned long, std::list<MieruCache::CacheEvent>>> event_vector;

static int prepareEventVector(std::ifstream &fin, int count) {
    for (size_t i = 0; i < count; i++) {
        unsigned long time;
        fin >> time;
        // Create a new item in the vector if the time is different
        if (event_vector.empty() || event_vector.back().first != time) {
            event_vector.emplace_back(time, std::list<MieruCache::CacheEvent>());
        }

        MieruCache::CacheEvent event;
        fin >> event.hart_id;
        fin >> event.initiator_id;
        fin >> event.index;
        fin >> event.address;
        fin >> event.old_state;
        fin >> event.new_state;
        event_vector.back().second.push_back(event);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << "\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Error: could not open file " << argv[1] << "\n";
        return 1;
    }

    int count, num_harts, num_entries, num_ways;
    fin >> count;
    num_harts = 4;      // TODO
    num_entries = 1024; // TODO
    num_ways = 1;       // TODO
    std::cout << "Input lines: " << count << "\n";
    prepareEventVector(fin, count);
    fin.close();

    Gtk::Main kit;
    MieruCache::MainWindow main_window(800, 600, num_harts, num_entries, num_ways, event_vector.size());
    kit.run(main_window);
    return 0;
}

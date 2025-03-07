#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <memory>
#include <sstream>
#include <gtkmm.h>

namespace MieruCache {
const std::string initial_message = "Initial state";
unsigned long initial_time;

struct CacheEvent {
    unsigned int hart_id;
    unsigned int initiator_id;
    unsigned int index;
    unsigned long address;
    char old_state;
    char new_state;
};

class MainWindow : public Gtk::Window {
    int num_harts, num_entries, num_ways;
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
    Gtk::Label time_label;
    Gtk::Toolbar toolbar;
    Gtk::Box box, toolbar_box;
    Gtk::Frame event_frame;
    Gtk::Label event_label;
    void onClickPrevButton() {
        if (time_slider.get_value() > 0) {
            time_slider.set_value(time_slider.get_value() - 1);
        }
    };
    void onClickNextButton() {
        if (time_slider.get_value() < time_slider.get_adjustment()->get_upper()) {
            time_slider.set_value(time_slider.get_value() + 1);
        }
    };
    void onChangeTimeSlider() { // TODO
        auto time_id = time_slider.get_value();
        showCache(time_id);
        showTime(time_id);
        if (time_id > 0) {
            showEvent(time_id - 1);
        } else {
            event_label.set_text(initial_message);
        }
    };
    void showCache(int time_id);
    void showTime(int time_id);
    void showEvent(int time_id);
public:
    MainWindow(int width, int height, int num_harts, int num_entries, int num_ways, int num_events) {
        std::cout << "Creating main window..." << std::flush;
        this->num_harts = num_harts;
        this->num_entries = num_entries;
        this->num_ways = num_ways;
        set_default_size(width, height);
        set_title("MieruCache");
        add_events(Gdk::BUTTON_PRESS_MASK);

        // Set up toolbar
        prev_button.set_icon_name("go-previous");
        prev_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onClickPrevButton));
        toolbar.append(prev_button);
        next_button.set_icon_name("go-next");
        next_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onClickNextButton));
        toolbar.append(next_button);
        toolbar.append(separator);

        time_label.set_text("Time: " + std::to_string(initial_time));

        // Set up time slider
        time_slider.set_range(0, num_events);
        time_slider.set_value(0);
        time_slider.set_digits(0);
        time_slider.set_increments(1, 1);
        time_slider.set_draw_value(false);
        time_slider.set_size_request(480, 0);
        time_slider.signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::onChangeTimeSlider));

        toolbar_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        toolbar_box.pack_start(toolbar, Gtk::PACK_SHRINK);
        toolbar_box.pack_start(time_slider, Gtk::PACK_SHRINK);
        toolbar_box.pack_start(time_label, Gtk::PACK_SHRINK);

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
        // Show cache at time 0
        showCache(0);
        // Add tree view to scrolled window
        scrolled_window.add(tree_view);

        // Add scrolled window to frame
        scrolled_window.set_border_width(2);
        caches_frame.add(scrolled_window);
        caches_frame.set_label("Caches");
        set_border_width(8);

        // Prepare event label
        event_label.set_text(initial_message);

        // Prepare event frame
        event_frame.set_label("Events");
        event_frame.add(event_label);
        set_border_width(8);

        // Put everything in a box
        box.set_orientation(Gtk::ORIENTATION_VERTICAL);
        // box.pack_start(time_slider, Gtk::PACK_SHRINK);
        box.pack_start(toolbar_box, Gtk::PACK_SHRINK);
        box.pack_start(caches_frame, Gtk::PACK_EXPAND_WIDGET);
        box.pack_start(event_frame, Gtk::PACK_SHRINK);
        add(box);

        show_all_children();
        std::cout << "done\n";
    }
    virtual ~MainWindow() = default;
};
}

static std::vector<std::vector<std::shared_ptr<std::pair<unsigned long, char>>>> cache;
static std::vector<std::pair<unsigned long, std::list<MieruCache::CacheEvent>>> event_vector;

void MieruCache::MainWindow::showCache(int time_id) {
    // Get the iterator to the first row
    auto row = *(list_store->children().begin());
    while (row) {
        auto idx = row[index];
        for (int i = 0; i < num_harts; i++) {
            for (int j = 0; j < num_ways; j++) {
                std::stringstream ss;
                auto cache_line = cache[time_id][num_harts * num_ways * idx + num_ways * i + j];
                if (cache_line->second == 'I') {
                    ss << "";
                } else {
                    ss << std::hex << cache_line->first << " (" << cache_line->second << ")";
                }
                row[cache_lines[i * num_ways + j]] = ss.str();
            }
        }
        row++;
    }
}

void MieruCache::MainWindow::showTime(int time_id) {
    time_label.set_text("Time: " + std::to_string((time_id == 0) ? initial_time : event_vector[time_id - 1].first));
}

void MieruCache::MainWindow::showEvent(int time_id) {
    std::stringstream ss;
    ss << "Event: Hart[" << event_vector[time_id].second.front().hart_id << "], " <<
        "Initiator[" << event_vector[time_id].second.front().initiator_id << "], " <<
        "Index[" << event_vector[time_id].second.front().index << "], " <<
        "Address[" << std::hex << event_vector[time_id].second.front().address << "], " <<
        "Old State[" << event_vector[time_id].second.front().old_state << "], " <<
        "New State[" << event_vector[time_id].second.front().new_state << "]";
    event_label.set_text(ss.str());
}

static int initializeCache(std::ifstream &fin, int num_harts, int num_entries, int num_ways) {
    std::cout << "Initializing cache..." << std::flush;
    cache.emplace_back(std::vector<std::shared_ptr<std::pair<unsigned long, char>>>(num_harts * num_entries * num_ways));
    for (int i = 0; i < num_harts; i++) {
        int hart_id;
        fin >> hart_id;
        for (int way = 0; way < num_ways; way++) {
            for (int index = 0; index < num_entries; index++) {
                std::string address;
                char state;
                fin >> address;
                fin >> state;
                cache[0][num_harts * num_ways * index + num_ways * hart_id + way] = std::make_shared<std::pair<unsigned long, char>>(
                    std::stoul(address, nullptr, 16), state);
            }
        }
    }
    std::cout << "done\n";

    return 0;
}

static int prepareEventVector(std::ifstream &fin) {
    std::cout << "Preparing event vector..." << std::flush;
    while (fin) {
        unsigned long time;
        fin >> time;
        // Create a new item in the vector if the time is different
        if (event_vector.empty() || event_vector.back().first != time) {
            event_vector.emplace_back(time, std::list<MieruCache::CacheEvent>());
        }

        MieruCache::CacheEvent event;
        std::string address;
        fin >> event.hart_id;
        fin >> event.initiator_id;
        fin >> event.index;
        fin >> address;
        try {
            event.address = std::stoul(address, nullptr, 16);
        } catch (std::invalid_argument &e) {
            break;
        }
        fin >> event.old_state;
        fin >> event.new_state;
        event_vector.back().second.push_back(event);
    }
    std::cout << "done\n";

    return 0;
}

static int generateCacheHistory(int num_harts, int num_ways) {
    std::cout << "Generating cache history..." << std::flush;
    int num_events = event_vector.size();
    // Resize the cache vector
    cache.resize(num_events + 1); // +1 for the initial state
    for (int i = 1; i <= num_events; i++) {
        // Copy the previous state
        cache[i] = cache[i - 1];
        // Apply the events
        for (auto &event : event_vector[i - 1].second) {
            unsigned int hart_id = event.hart_id;
            unsigned int index = event.index;
            unsigned int way = 0; // TODO

            cache[i][num_harts * num_ways * index + num_ways * hart_id + way] = std::make_shared<std::pair<unsigned long, char>>(event.address, event.new_state);
        }
    }
    std::cout << "done\n";

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
    fin >> num_harts;
    fin >> num_entries;
    fin >> num_ways;
    fin >> MieruCache::initial_time;
    initializeCache(fin, num_harts, num_entries, num_ways);
    prepareEventVector(fin);
    std::cout << "Closing file " << argv[1] << "..." << std::flush;
    fin.close();
    std::cout << "done\n";
    generateCacheHistory(num_harts, num_ways);

    Gtk::Main kit;
    MieruCache::MainWindow main_window(800, 600, num_harts, num_entries, num_ways, event_vector.size());
    kit.run(main_window);
    return 0;
}

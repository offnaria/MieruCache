#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <list>
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
public:
    MainWindow(int width, int height) {
        set_default_size(width, height);
        set_title("MieruCache");
    }
    virtual ~MainWindow() = default;
};
}

static std::map<unsigned long, std::list<MieruCache::CacheEvent>> event_map;

static int prepareEventMap(std::ifstream &fin, int count) {
    for (size_t i = 0; i < count; i++) {
        unsigned long time;
        fin >> time;
        if (event_map.find(time) == event_map.end()) {
            event_map[time] = std::list<MieruCache::CacheEvent>();
        }

        MieruCache::CacheEvent event;
        fin >> event.hart_id;
        fin >> event.initiator_id;
        fin >> event.index;
        fin >> event.address;
        fin >> event.old_state;
        fin >> event.new_state;
        event_map[time].push_back(event);
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

    int count;
    fin >> count;
    std::cout << "Input lines: " << count << "\n";
    prepareEventMap(fin, count);
    fin.close();

    for (auto& [time, event_list] : event_map) {
        std::cout << time << "\n";
        for (auto& event : event_list) {
            std::cout << "\t" << event.hart_id << " " << event.initiator_id << " " << event.index << " " << event.address << " " << event.old_state << " " << event.new_state << "\n";
        }
    }

    Gtk::Main kit(argc, argv);
    MieruCache::MainWindow main_window(800, 600);
    kit.run(main_window);
    return 0;
}

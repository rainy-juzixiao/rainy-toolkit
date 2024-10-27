#include <functional>
#include <iostream>
#include <shared_mutex>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rainy::foundation::event {
	struct index {
		std::string make_index() noexcept {
			return uuid + "." + event_name;
		}

		bool is_empty() const noexcept {
			return uuid.empty() && event_name.empty();
		}

		std::string uuid;
		std::string event_name;
	};

	struct arguments {
		operator bool() const {
			return index.is_empty() && your_data.has_value();
		}

		index index;
		containers::any your_data;
	};

	class event_base {
	public:
		event_base(const event_base &) = delete;
		event_base(event_base &&) = delete;
		event_base &operator=(const event_base &) = delete;
		event_base &operator=(event_base &&) = delete;

		virtual ~event_base() = default;

		using callback_class_t = std::unordered_map<std::string, std::vector<std::function<void(arguments)>>>;

		static void dispatch_to_system(const std::string &uuid, const std::string &event, const arguments &data) {
			auto &ref = *global_data();
			if (uuid != "GLOBAL") {
				auto find = ref.find(uuid);
				if (find == ref.end()) {
					return; // Not registered
				}
				auto callbacks = find->second.find(event);
				if (callbacks == find->second.end()) {
					return; // No callbacks found
				}
			} else {
				for (auto &callback: ref["GLOBAL"]["CALLBACKS"]) {
					try {
						callback(data);
					} catch (const std::exception &e) {
						std::cerr << "Exception in global callback: " << e.what() << std::endl;
					}
				}
				return;
			}
			try {
				std::unique_ptr<arguments> actual_arguments(new arguments{{uuid, event}, data.your_data});
				for (const auto &cb: ref[uuid][event]) {
					cb(*actual_arguments);
				}
				index idx{uuid, event};
				actual_arguments->index.event_name = idx.make_index();
				actual_arguments->index.uuid = idx.uuid;
				dispatch_to_system("GLOBAL", idx.make_index(), *actual_arguments);
				// 递归调用，目标为GLOBAL，通过GLOBAL调用全局回调，并将事件的uuid和名称进行更新
			} catch (const std::exception &e) {
				std::cerr << "Exception in callback: " << e.what() << std::endl;
			}
		}

		static void dispatch_to_system(const event_base *instance, const std::string &event, const arguments &data) {
			dispatch_to_system(instance->get_class_uuid(), event, data);
		}

		static void display_system_mapping(std::ostream &ostream) noexcept {
			const auto &ref = *global_data();
			for (const auto &uuid: ref) {
				ostream << "UUID: " << uuid.first << "\n\tCallbacks: \n";
				for (const auto &callbacks: uuid.second) {
					ostream << "\tEvent: " << callbacks.first << "\n";
					int i = 1;
					for (const auto &address: callbacks.second) {
						ostream << "\t\t#" << i++ << ": " << &address << "\n";
					}
				}
			}
		}

		virtual std::string get_class_uuid() const = 0;

		virtual callback_class_t get_callback() const = 0;

		virtual void dispatch(const std::string &callback,const arguments& data) const = 0;

		static void add_global_callback(const std::function<void(arguments)> &callback) {
			auto &ref = *global_data();
			ref["GLOBAL"]["CALLBACKS"].emplace_back(callback);
		}

	protected:
		event_base() : mapping(global_data()) {
		}

		void initialize_from_derived(const event_base &derived) {
			static const std::regex regex(R"(^\{?[A-Fa-f0-9]{8}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{12}\}?$)",
										  std::regex::icase);
			auto find = mapping->find(derived.get_class_uuid());
			if (find != mapping->end()) {
				return; // Already registered
			}
			if (std::regex_match(derived.get_class_uuid(), regex)) {
				auto callbacks = derived.get_callback();
				mapping->emplace(derived.get_class_uuid(), callbacks);
			}
		}

	private:
		static std::unordered_map<std::string, callback_class_t> *global_data() noexcept {
			static std::unordered_map<std::string, callback_class_t> global;
			static std::shared_mutex mtx;
			std::shared_lock<std::shared_mutex> lock(mtx);
			return &global;
		}

		static std::vector<std::function<void(arguments)>> &global_callbacks() noexcept {
			static std::vector<std::function<void(arguments)>> global_callbacks;
			static std::shared_mutex mtx;
			std::shared_lock<std::shared_mutex> lock(mtx);
			return global_callbacks;
		}

		std::unordered_map<std::string, callback_class_t> *mapping;
	};

	class test_event : public event_base {
	public:
		test_event() {
			initialize_from_derived(*this);
		}

		std::string get_class_uuid() const override {
			return "F68A783D-E95F-4446-B061-77982021F31C";
		}

		callback_class_t get_callback() const override {
			return {{"test", {&test}}};
		}

		static void test(arguments args) {
			std::cout << "Hello World\n";
		}

		void dispatch(const std::string &event, const arguments &data) const override {
			this->dispatch_to_system(get_class_uuid(), event, data);
		}
	};
}

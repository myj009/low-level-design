#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <stdexcept>

// Forward declarations
class Bookmark;
class BookmarkFolder;

// Interface for browser-specific bookmark importers
class IBookmarkImporter {
public:
    virtual std::vector<std::unique_ptr<Bookmark>> importBookmarks() = 0;
    virtual ~IBookmarkImporter() = default;
};

// Interface for bookmark export strategies
class IBookmarkExporter {
public:
    virtual void exportBookmarks(const std::vector<std::unique_ptr<Bookmark>>& bookmarks) = 0;
    virtual ~IBookmarkExporter() = default;
};

// Interface for search strategies
class ISearchStrategy {
public:
    virtual std::vector<Bookmark*> search(const std::string& query,
                                        const std::vector<std::unique_ptr<Bookmark>>& bookmarks) = 0;
    virtual ~ISearchStrategy() = default;
};

// Base class for bookmark entries (Component pattern)
class BookmarkEntry {
public:
    BookmarkEntry(const std::string& name) : name_(name) {}
    virtual ~BookmarkEntry() = default;

    virtual void display() const = 0;
    
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

protected:
    std::string name_;
};

// Concrete bookmark class
class Bookmark : public BookmarkEntry {
public:
    Bookmark(const std::string& name, const std::string& url, const std::string& browser)
        : BookmarkEntry(name), url_(url), browser_(browser) {
        timestamp_ = std::chrono::system_clock::now();
    }

    void display() const override {
        // Implementation for displaying bookmark details
    }

    const std::string& getUrl() const { return url_; }
    void setUrl(const std::string& url) { url_ = url; }
    
    const std::string& getBrowser() const { return browser_; }
    const auto& getTimestamp() const { return timestamp_; }

private:
    std::string url_;
    std::string browser_;
    std::chrono::system_clock::time_point timestamp_;
};

// Folder class (Composite pattern)
class BookmarkFolder : public BookmarkEntry {
public:
    BookmarkFolder(const std::string& name) : BookmarkEntry(name) {}

    void addEntry(std::unique_ptr<BookmarkEntry> entry) {
        entries_.push_back(std::move(entry));
    }

    void removeEntry(const std::string& name) {
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                [&name](const auto& entry) { return entry->getName() == name; }),
            entries_.end()
        );
    }

    void display() const override {
        // Implementation for displaying folder contents
    }

    const std::vector<std::unique_ptr<BookmarkEntry>>& getEntries() const {
        return entries_;
    }

private:
    std::vector<std::unique_ptr<BookmarkEntry>> entries_;
};

// Concrete browser-specific importers
class ChromeImporter : public IBookmarkImporter {
public:
    std::vector<std::unique_ptr<Bookmark>> importBookmarks() override {
        // Implementation for importing Chrome bookmarks
        return std::vector<std::unique_ptr<Bookmark>>();
    }
};

class FirefoxImporter : public IBookmarkImporter {
public:
    std::vector<std::unique_ptr<Bookmark>> importBookmarks() override {
        // Implementation for importing Firefox bookmarks
        return std::vector<std::unique_ptr<Bookmark>>();
    }
};

// Concrete search strategy
class DefaultSearchStrategy : public ISearchStrategy {
public:
    std::vector<Bookmark*> search(const std::string& query,
                                const std::vector<std::unique_ptr<Bookmark>>& bookmarks) override {
        std::vector<Bookmark*> results;
        for (const auto& bookmark : bookmarks) {
            if (bookmark->getName().find(query) != std::string::npos ||
                bookmark->getUrl().find(query) != std::string::npos) {
                results.push_back(bookmark.get());
            }
        }
        return results;
    }
};

// Observer interface for bookmark changes
class IBookmarkObserver {
public:
    virtual void onBookmarkAdded(const Bookmark& bookmark) = 0;
    virtual void onBookmarkRemoved(const Bookmark& bookmark) = 0;
    virtual void onBookmarkUpdated(const Bookmark& bookmark) = 0;
    virtual ~IBookmarkObserver() = default;
};

// Main UBMS class (Facade pattern)
class UnifiedBookmarkManagementSystem {
public:
    UnifiedBookmarkManagementSystem() 
        : searchStrategy_(std::make_unique<DefaultSearchStrategy>()) {
        rootFolder_ = std::make_unique<BookmarkFolder>("Root");
    }

    void registerImporter(const std::string& browserName, 
            std::unique_ptr<IBookmarkImporter> importer) {
        importers_[browserName] = std::move(importer);
    }

    void importBookmarks(const std::string& browserName) {
        auto it = importers_.find(browserName);
        if (it == importers_.end()) {
            throw std::runtime_error("No importer found for browser: " + browserName);
        }

        auto bookmarks = it->second->importBookmarks();
        for (auto& bookmark : bookmarks) {
            addBookmark(std::move(bookmark));
        }
    }

    void addBookmark(std::unique_ptr<Bookmark> bookmark) {
        rootFolder_->addEntry(std::move(bookmark));
        notifyObservers([&](auto& observer) {
            observer.onBookmarkAdded(*static_cast<Bookmark*>(
                rootFolder_->getEntries().back().get()));
        });
    }

    void removeBookmark(const std::string& name) {
        auto bookmark = findBookmark(name);
        if (bookmark) {
            notifyObservers([&](auto& observer) {
                observer.onBookmarkRemoved(*bookmark);
            });
            rootFolder_->removeEntry(name);
        }
    }

    std::vector<Bookmark*> search(const std::string& query) {
        return searchStrategy_->search(query, getAllBookmarks());
    }

    void addObserver(IBookmarkObserver* observer) {
        observers_.push_back(observer);
    }

    void removeObserver(IBookmarkObserver* observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }

private:
    std::vector<std::unique_ptr<Bookmark>> getAllBookmarks() const {
        std::vector<std::unique_ptr<Bookmark>> bookmarks;
        // Implementation to collect all bookmarks recursively
        return bookmarks;
    }

    Bookmark* findBookmark(const std::string& name) {
        // Implementation to find bookmark by name
        return nullptr;
    }

    template<typename F>
    void notifyObservers(F notification) {
        for (auto* observer : observers_) {
            notification(*observer);
        }
    }

    std::unique_ptr<BookmarkFolder> rootFolder_;
    std::unordered_map<std::string, std::unique_ptr<IBookmarkImporter>> importers_;
    std::unique_ptr<ISearchStrategy> searchStrategy_;
    std::vector<IBookmarkObserver*> observers_;
};
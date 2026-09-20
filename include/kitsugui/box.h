#ifndef KITSUGUI_BOX_H
#define KITSUGUI_BOX_H

#include "kitsugui/widget.h"
#include <vector>

namespace KitsuGui {

class KitsuBox : public KitsuWidget {
public:
    KitsuBox(bool horizontal = true);
    ~KitsuBox() override;
    
    void setAlignment(KitsuAlign a)   { align = a; markDirty(); }
    void setJustify(KitsuJustify j)   { justify_content = j; markDirty(); }
    void setSpacing(int s)            { spacing = s; markDirty(); }
    
    KitsuBox& setMargin(int m)        { margin = m; markDirty(); return *this; }
    KitsuBox& setPadding(int p)       { padding = p; markDirty(); return *this; }
    
    KitsuBox& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    void setAutoLayout(bool enabled)  { auto_layout = enabled; markDirty(); }
    bool isAutoLayout() const         { return auto_layout; }
    
    void addChild(KitsuWidget* child, bool owns = true);
    void removeChild(KitsuWidget* child);
    void clearChildren();
    
    void updateLayout() override;
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;
    
    const std::vector<KitsuWidget*>& getChildren() const { return children; }
    bool isHorizontal() const { return horizontal; }
    
    // Getters para ScrollView u otros que necesiten consultar
    int getSpacing() const { return spacing; }
    int getPadding() const { return padding; }
    
protected:
    bool horizontal = true;
    bool auto_layout = true;
    KitsuAlign align = KitsuAlign::START;
    KitsuJustify justify_content = KitsuJustify::START;
    int spacing = 0;
    std::vector<KitsuWidget*> children;
    std::vector<KitsuWidget*> owned_children;
    
    void layoutHorizontal();
    void layoutVertical();
};

using KitsuHBox = KitsuBox;
using KitsuVBox = KitsuBox;

} // namespace KitsuGui

#endif

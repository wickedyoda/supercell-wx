# Touch UI Adaptation Strategy for Android

## Overview

Supercell Wx is being adapted to provide an excellent touch experience on Android phones and tablets. This document outlines the touch interaction patterns and responsive layout strategy.

## Touch Interaction Patterns

### Map Interactions (MapWidget)

| Gesture | Desktop | Android |
|---------|---------|---------|
| Pan | Left-drag | Single-finger drag |
| Zoom in | Scroll wheel up / Double-click | Pinch (two-finger) / Double-tap |
| Zoom out | Scroll wheel down / Double-click | Pinch (two-finger) / Double-tap |
| Rotate | Right-drag | Two-finger rotation (future) |
| Select radar | Middle-click | Long-press / Tap radar icon |
| Change style | Left+Right click | Gesture button (future) |

**Status**: Pinch and single-finger drag already work. Additions needed:
- Double-tap zoom
- Long-press for radar selection
- Improved touch target size for radar site icons

### Menu and Button Interactions

| Element | Desktop | Android |
|---------|---------|---------|
| Menus | Menu bar (File, Tools, etc.) | Action bar + overflow menu |
| Buttons | Click | Tap |
| Button size | ~32x32 px | **44x44 dp (minimum)** |
| Toggles | Click | Tap + visual feedback |
| Sliders | Drag | Drag (larger track height) |

**Status**: Current Qt UI uses standard button sizes. Additions needed:
- Increase button padding to 12-16 dp
- Use Material Design touch targets (48x48 dp recommended)
- Make menu bar collapsible on small screens
- Use side drawer for secondary menus on phones

## Responsive Layout Strategy

### Screen Size Breakpoints

```
Phone (< 600 dp):
  - Portrait mode primary
  - Single-column layout
  - Bottom action bar for map controls
  - Side drawer for settings/menus
  
Tablet (600-1000 dp):
  - Portrait or landscape
  - Two-column layout (controls + map)
  - Compact toolbar
  - Side panels for properties/details
  
Large Tablet (>1000 dp):
  - Landscape optimized
  - Multi-panel layout
  - Full-size controls
```

### Adaptive Layouts

**Current state**: Fixed desktop layout using Qt Widgets + QMainWindow menu bar.

**Adaptations needed**:

1. **Main Window Layout**
   - Detect screen size and orientation at runtime
   - Collapse menu bar to hamburger icon on phones
   - Use responsive dock widgets or side drawers

2. **Control Panels**
   - Reduce padding/margins on phones (12 dp vs 24 dp on desktop)
   - Stack panels vertically on portrait mode
   - Use collapsible sections to save space

3. **Color Table and Legend**
   - Floating panel on desktop (always visible)
   - Slide-in/modal on phones (swipe from edge or tap button)

4. **Radar Product Selector**
   - Dropdown/combobox on desktop
   - Bottom sheet or modal picker on phones

5. **Layer Manager**
   - Dock widget on desktop (left/right side)
   - Modal dialog or drawer on phones

## Implementation Priorities

### Phase 1: Core Touch Gestures (Current)
- ✅ Pinch zoom (already implemented)
- ⬜ Double-tap zoom
- ⬜ Long-press radar selection
- ⬜ Swipe navigation (optional)

### Phase 2: Button & Control Sizing
- ⬜ Increase touch target sizes to 44-48 dp
- ⬜ Add visual feedback (ripple effects, state indicators)
- ⬜ Test on real devices with typical hand sizes

### Phase 3: Layout Adaptation
- ⬜ Implement orientation detection
- ⬜ Collapsible menu bar (hamburger menu)
- ⬜ Responsive dock widget positioning
- ⬜ Bottom sheet or modal dialogs for phone UIs

### Phase 4: Mobile-Specific Features
- ⬜ Swipe between saved radar sites
- ⬜ Quick access favorites
- ⬜ Voice commands for navigation
- ⬜ Notification integration for alerts

## Code Changes Needed

### 1. MapWidget: Enhanced Touch Gestures

**File**: `scwx-qt/source/scwx/qt/map/map_widget.cpp`

```cpp
// Add double-tap zoom
void MapWidget::gestureEvent(QGestureEvent* ev) {
    // Existing pinch logic...
    
    // Add double-tap zoom
    if (auto* tap = dynamic_cast<QTapGesture*>(ev->gesture(Qt::TapGesture))) {
        p->HandleDoubleTap(tap);
    }
}

// Add long-press for radar selection
void MapWidget::gestureEvent(QGestureEvent* ev) {
    if (auto* press = dynamic_cast<QTapAndHoldGesture*>(ev->gesture(Qt::TapAndHoldGesture))) {
        p->HandleLongPress(press);
    }
}
```

### 2. MainWindow: Responsive Menu Bar

**File**: `scwx-qt/source/scwx/qt/main/main_window.cpp`

```cpp
// Detect screen size and adapt menu visibility
void MainWindow::showEvent(QShowEvent* event) {
    QScreen* screen = this->screen();
    int screenWidth = screen->geometry().width();
    
    if (screenWidth < 600) {
        // Phone: use hamburger menu
        p->EnableHamburgerMenu();
    } else {
        // Tablet/Desktop: use full menu bar
        p->EnableFullMenuBar();
    }
}

// Handle orientation changes
void MainWindow::resizeEvent(QResizeEvent* event) {
    if (this->width() < 600 && this->width() > this->height()) {
        // Landscape phone: adjust layout
        p->AdaptForLandscapePhone();
    }
}
```

### 3. Button Sizing

**File**: `scwx-qt/scwx-qt.ui` (Qt Designer)

Current approach uses fixed button sizes. For mobile:

```cpp
// Programmatic approach (in main_window.cpp):
const int TOUCH_TARGET_SIZE = 48; // dp, ~48x48 pixels

auto* button = new QPushButton("Menu");
button->setMinimumSize(TOUCH_TARGET_SIZE, TOUCH_TARGET_SIZE);
button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
```

Or use stylesheets:

```qss
QPushButton {
    min-width: 48px;
    min-height: 48px;
    padding: 12px;
    font-size: 14pt;
}
```

## Testing Strategy

### Device Targets
- **Phone (5.5-6.5 in)**: Pixel 6/7 (6.3"), Moto G Power (6.5")
- **Tablet (7-10 in)**: iPad mini (7.9"), Samsung Tab S6 (10.5")

### Gestures to Test
1. Single-finger pan (left/right/up/down)
2. Pinch zoom (two fingers)
3. Pinch rotate (two fingers rotated)
4. Double-tap (zoom)
5. Long-press (context menu)
6. Swipe between screens (if implemented)

### Layout Testing
- Portrait orientation
- Landscape orientation
- Screen size transitions (rotation)
- High-DPI screens (xhdpi, xxhdpi, xxxhdpi)

## Performance Considerations

1. **Touch Latency**: Minimize input-to-render latency for smooth interactions
2. **Battery**: Heavy rendering (map, overlay layers) impacts battery; optimize frame rate
3. **Memory**: Android devices have varying RAM; monitor texture atlases and layer caching
4. **Network**: Radar tile downloads must be efficient; consider offline caching

## Accessibility

1. **Touch Targets**: All interactive elements 44-48 dp minimum
2. **Contrast**: Ensure sufficient color contrast for outdoor visibility
3. **Text Size**: Respect system font scaling preferences
4. **Haptic Feedback**: Consider vibration feedback for actions (future)

## References

- [Android Design Guidelines](https://developer.android.com/design)
- [Material Design Touch Targets](https://material.io/design/usability/accessibility.html#layout-typography)
- [Qt for Android Gestures](https://doc.qt.io/qt-6/qgesture.html)
- [Qt Size Guidelines](https://doc.qt.io/qt-6/scalable-user-interfaces.html)

## Next Steps

1. Implement double-tap zoom (MapWidget)
2. Implement long-press radar selection (MapWidget)
3. Add button size detection and adaptive styling (MainWindow)
4. Create layout adaptation for orientation changes
5. Test on emulator and real devices

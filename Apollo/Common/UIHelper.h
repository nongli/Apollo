#pragma once

#include <string>
#include "Types.h"
#include "Color4f.h"
#include "Timer.h"
#include "Logging.h"

namespace Apollo {   
    static const char* TABS[] = {
        "",
        "    ",
        "        ",
        "            ",
        "                "
    };
    static const UINT NUM_TAB_ENTRIES = sizeof(TABS)/sizeof(TABS[0]);

    class UIHelper {
    public:
        static Color4f GetColorIntensity(FLOAT value);
        static std::string DisplayTime(UINT32 ms);
    };

    class TaskData {
    public:
        TaskData(std::string name, UINT work, UINT update, bool bDisplayTimeAtEnd, const TaskData* parent = nullptr) : 
                m_name(name), 
                m_totalWork(work), 
                m_updateInterval(update), 
                m_printTimeOnFinish(bDisplayTimeAtEnd),
                m_parent(parent),
                timer(),
                m_percentComplete(0),
                m_lastUpdate(0),
                m_started(false) {
            if (m_parent) {
                m_tabs = m_parent->GetTabs() + 1;
                m_tabs = CLAMP(m_tabs, 0, NUM_TAB_ENTRIES - 1);
            } else {
                m_tabs = 0;
            }
                        
            timer.Start();
            SetProgress(0);
        }

        void SetProgress(UINT progress) { 
            m_progress = progress; 
            Update();
        }

        void Complete() {
            if (m_progress == m_totalWork) return;
            SetProgress(m_totalWork);
        }

    protected:
        UINT GetTabs() const { return m_tabs; }

    private:
        void Update()
        {
            ILogger* logger = ILogger::Logger();
            if (!m_started) {
                ILogger::Logger()->Status("%sStarting \"%s\" ...", TABS[m_tabs], m_name.c_str());
                m_started = true;
            }

            if (m_progress == m_totalWork) {
                timer.Stop();
                if (m_printTimeOnFinish) {
                    logger->Status("%s    Completed [%s].", TABS[m_tabs],
                        UIHelper::DisplayTime(timer.Stop()).c_str());
                } else {
                    logger->Status("%s    Completed.", TABS[m_tabs]);
                }
                return;
            }
            
            if (m_progress == 0) return;
            if (m_updateInterval == 0) return;
            
            m_percentComplete = m_progress * 100 / m_totalWork;
            if (m_percentComplete / m_updateInterval != m_lastUpdate) {
                m_lastUpdate = m_percentComplete / m_updateInterval;
                logger->Status("%s    %d%% complete...", TABS[m_tabs], m_lastUpdate*m_updateInterval);
            }
        }

        std::string m_name;
        UINT m_totalWork;
        UINT m_updateInterval;
        UINT m_progress;
        bool m_printTimeOnFinish;
        Timer timer;
        const TaskData* m_parent;
        UINT m_tabs;
        UINT m_percentComplete;
        UINT m_lastUpdate;
        bool m_started;
    };
}

#include "App.h"
#include "imgui.h"
#include "implot.h"
#include "audio.hpp"

// Buffer size and sample rate
static const int N = 8'192;
static const double fs = 8'000.0;

// Filter parameters
static double fc = 350.0;

namespace ImGui {

static inline bool SliderDouble(const char *label, double *v, double v_min, double v_max, const char *format = NULL, ImGuiSliderFlags flags = 0) {
    return SliderScalar(label, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
}

static inline bool SliderInt32(const char *label, int *v, int v_min, int v_max, const char *format = NULL, ImGuiSliderFlags flags = 0) {
    return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

static inline float GetWindowContentRegionWidth() {
    return GetWindowContentRegionMax().x - GetWindowContentRegionMin().x;
}

} // namespace ImGui

struct Tuner : public App {
    using App::App;

    void Update() override {
        constexpr double min_fc = 5.0;
        constexpr double max_fc = 500.0;

        static Audio audio(N, fs, fc);

        // gui inputs
        static bool etfe_need_update = true;
        static bool filter_need_update = false;

        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(GetWindowSize(), ImGuiCond_Always);
        ImGui::Begin("Filter", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, -1));

        ImGui::Text("Filter parameters");
        if (ImGui::SliderDouble("Cutoff frequency", &fc, min_fc, max_fc, "%.3f"))
            filter_need_update = true;
        ImGui::Separator();

        if (filter_need_update) {
            fc = std::clamp(fc, min_fc, max_fc);
            audio.setup_filter(fs, fc);
        }

        const Audio::Result& audio_res = audio.get_buffer();

        // plot waveforms
        if (ImPlot::BeginPlot("##Filter", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("Buffer Frames","Signal");
            ImPlot::SetupAxesLimits(0, N, -1, 1);

            ImPlot::SetupLegend(ImPlotLocation_NorthEast);
            ImPlot::PlotLine("Input Signal", audio_res.raw.data(), N);
            ImPlot::PlotLine("Filtered Signal", audio_res.flt.data(), N);
            ImPlot::EndPlot();
        }
        ImGui::EndChild(); // ChildL
        ImGui::SameLine();
        ImGui::BeginChild("ChildR", ImVec2(0, -1));

        // perform ETFE
        static int window         = 0;
        static int inwindow       = 3;
        static int nwindow_opts[] = {100, 200, 500, 1'000, 2'000, 5'000, 10'000};
        static int infft          = 6;
        static int nfft_opts[]    = {100, 200, 500, 1'000, 2'000, 5'000, 10'000, 20'000, 50'000};
        static float overlap      = 0.5f;

        ImGui::Text("Frequency Response");
        ImGui::Separator();
        if (ImGui::Combo("FFT Size", &infft, "100\0""200\0""500\0""1000\0""2000\0""5000\0""10000\0""20000\0""50000\0")) {
            inwindow = infft < inwindow ? infft : inwindow;
            etfe_need_update = true;
        }

        if (ImGui::Combo("Window Type", &window, "hamming\0hann\0winrect\0"))
            etfe_need_update = true;
        if (ImGui::Combo("Window Size", &inwindow,"100\0""200\0""500\0""1000\0""2000\0""5000\0""10000\0"))
            etfe_need_update = true;
        if (ImGui::SliderFloat("Window Overlap", &overlap, 0, 1, "%.2f"))
            etfe_need_update = true;

        ImGui::NewLine();

        if (etfe_need_update) {
            infft = inwindow > infft ? inwindow : infft;
            int nwindow  = nwindow_opts[inwindow];
            int noverlap = (int)(nwindow * overlap);
            int nfft     = nfft_opts[infft];
            audio.setup_etfe(
                N, fs,
                window == 0 ? etfe::hamming(nwindow)
                    : window == 1 ? etfe::hann(nwindow)
                    : etfe::winrect(nwindow),
                noverlap, nfft
            );
            etfe_need_update = false;
        }

        if (ImGui::BeginTable("table", 4)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Current frequency");
            ImGui::TableNextColumn();
            ImGui::Text("Nearest note frequency");
            ImGui::TableNextColumn();
            ImGui::Text("Nearest note name");
            ImGui::TableNextColumn();
            ImGui::Text("Accuracy");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", audio_res.etfe.f[audio_res.etfe.pidx]);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", audio_res.pd.f);
            ImGui::TableNextColumn();
            ImGui::Text("%d%s", audio_res.pd.octave, audio_res.pd.note);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", audio_res.etfe.df);
            ImGui::EndTable();
        }
        if (audio_res.pd.in_tune) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("In tune!");
            ImGui::PopStyleColor();
        } else {
            if (audio_res.pd.cents > 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                ImGui::Text("Cents sharp: %.2f", audio_res.pd.cents);
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                ImGui::Text("Cents flat: %.2f", -audio_res.pd.cents);
                ImGui::PopStyleColor();
            }
        }
        ImGui::Separator();

        if (ImPlot::BeginPlot("##Amp", ImVec2(-1, -1))) {
            ImPlot::SetupAxesLimits(0, 355, 0, 0.05);
            ImPlot::SetupAxes("Frequency [Hz]", "Amplitude [dB]");
            ImPlot::SetupLegend(ImPlotLocation_NorthEast);
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.25f);
            ImPlot::PlotShaded("x[f]", audio_res.etfe.f.data(), audio_res.etfe.ampy.data(), (int)audio_res.etfe.f.size(), -INFINITY);
            ImPlot::PlotLine("x[f]", audio_res.etfe.f.data(), audio_res.etfe.ampy.data(), (int)audio_res.etfe.f.size());
            ImPlot::TagX(audio_res.etfe.f[audio_res.etfe.pidx], ImVec4(1,1,1,0.9), "%d%s: %.1f cents", audio_res.pd.octave, audio_res.pd.note, audio_res.pd.cents);
            if (ImPlot::DragLineX(397391, &fc, ImVec4(.15f, .15f, .15f, 1))) {
                filter_need_update = true;
            }
            ImPlot::EndPlot();
        }

        ImGui::EndChild();
        ImGui::End();
    }
};

int main(int argc, char const *argv[]) {
    Tuner app("ImFilter", 960, 540, argc, argv);
    app.Run();

    return 0;
}

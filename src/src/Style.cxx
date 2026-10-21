/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include "../include/cad_common/Style.hxx"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QFile>
#include <QGuiApplication>
#include <QIODevice>
#include <QPalette>
#include <QString>
#include <QStyle>
#include <QStyleFactory>

#include <map>
#include <string>

// Must live at global scope: Q_INIT_RESOURCE declares qInitResources_<name>
static void initStyleResources() {
    Q_INIT_RESOURCE(styles);
}

namespace {

using Tokens = std::map<std::string, QString>;

const Tokens& lightTokens() {
    static const Tokens tokens = {
        {"window", QStringLiteral("#f6f7f9")},
        {"windowText", QStringLiteral("#1f2328")},
        {"base", QStringLiteral("#ffffff")},
        {"alternateBase", QStringLiteral("#eef1f4")},
        {"text", QStringLiteral("#1f2328")},
        {"button", QStringLiteral("#ffffff")},
        {"buttonText", QStringLiteral("#1f2328")},
        {"highlight", QStringLiteral("#2f6feb")},
        {"highlightedText", QStringLiteral("#ffffff")},
        {"highlightHover", QStringLiteral("#4f8ef7")},
        {"highlightPressed", QStringLiteral("#1f5fd0")},
        {"mid", QStringLiteral("#d0d7de")},
        {"midlight", QStringLiteral("#eef1f5")},
        {"dark", QStringLiteral("#8c959f")},
        {"light", QStringLiteral("#ffffff")},
        {"placeholderText", QStringLiteral("#6e7781")},
        {"border", QStringLiteral("#d8dee4")},
        {"handle", QStringLiteral("#e6e9ee")},
        {"header", QStringLiteral("#f3f5f8")},
        {"central", QStringLiteral("#eef1f4")},
        {"radius", QStringLiteral("6px")},
        {"smallRadius", QStringLiteral("4px")},
        {"dangerHover", QStringLiteral("#cf222e")},
        {"dangerPressed", QStringLiteral("#a40e26")},
    };
    return tokens;
}

const Tokens& darkTokens() {
    static const Tokens tokens = {
        {"window", QStringLiteral("#1b1c20")},
        {"windowText", QStringLiteral("#e6e8eb")},
        {"base", QStringLiteral("#25272c")},
        {"alternateBase", QStringLiteral("#2c2f35")},
        {"text", QStringLiteral("#e6e8eb")},
        {"button", QStringLiteral("#2c2f35")},
        {"buttonText", QStringLiteral("#e6e8eb")},
        {"highlight", QStringLiteral("#4f8ef7")},
        {"highlightedText", QStringLiteral("#ffffff")},
        {"highlightHover", QStringLiteral("#6aa1f8")},
        {"highlightPressed", QStringLiteral("#3a7ae0")},
        {"mid", QStringLiteral("#3a3e45")},
        {"midlight", QStringLiteral("#303339")},
        {"dark", QStringLiteral("#131418")},
        {"light", QStringLiteral("#4a4e56")},
        {"placeholderText", QStringLiteral("#8b949e")},
        {"border", QStringLiteral("#363a41")},
        {"handle", QStringLiteral("#26292e")},
        {"header", QStringLiteral("#2b2e34")},
        {"central", QStringLiteral("#14151a")},
        {"radius", QStringLiteral("6px")},
        {"smallRadius", QStringLiteral("4px")},
        {"dangerHover", QStringLiteral("#e5534b")},
        {"dangerPressed", QStringLiteral("#b62324")},
    };
    return tokens;
}

QString tokenValue(const Tokens& tokens, const std::string& key) {
    const auto it = tokens.find(key);
    return it != tokens.end() ? it->second : QString();
}

QPalette makePalette(const Tokens& tokens) {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(tokenValue(tokens, "window")));
    palette.setColor(QPalette::WindowText, QColor(tokenValue(tokens, "windowText")));
    palette.setColor(QPalette::Base, QColor(tokenValue(tokens, "base")));
    palette.setColor(QPalette::AlternateBase, QColor(tokenValue(tokens, "alternateBase")));
    palette.setColor(QPalette::Text, QColor(tokenValue(tokens, "text")));
    palette.setColor(QPalette::Button, QColor(tokenValue(tokens, "button")));
    palette.setColor(QPalette::ButtonText, QColor(tokenValue(tokens, "buttonText")));
    palette.setColor(QPalette::Highlight, QColor(tokenValue(tokens, "highlight")));
    palette.setColor(QPalette::HighlightedText, QColor(tokenValue(tokens, "highlightedText")));
    palette.setColor(QPalette::Mid, QColor(tokenValue(tokens, "mid")));
    palette.setColor(QPalette::Midlight, QColor(tokenValue(tokens, "midlight")));
    palette.setColor(QPalette::Dark, QColor(tokenValue(tokens, "dark")));
    palette.setColor(QPalette::Light, QColor(tokenValue(tokens, "light")));
    palette.setColor(QPalette::PlaceholderText, QColor(tokenValue(tokens, "placeholderText")));
    return palette;
}

/// @brief Replace every @c {{token}} placeholder in @p qss with its value
QString substituteTokens(QString qss, const Tokens& tokens) {
    for (const auto& [key, value] : tokens) {
        qss.replace(QStringLiteral("{{%1}}").arg(QString::fromStdString(key)), value);
    }
    return qss;
}

cad_common::StyleMode resolveMode(const cad_common::StyleMode mode) {
    if (mode != cad_common::StyleMode::System) {
        return mode;
    }
    const int lightness = QGuiApplication::palette().color(QPalette::Window).lightness();
    return lightness < 128 ? cad_common::StyleMode::Dark : cad_common::StyleMode::Light;
}

} // namespace

namespace cad_common::style {

void apply(const StyleMode mode) {
    auto* app = qobject_cast<QApplication*>(QCoreApplication::instance());
    if (app == nullptr) {
        return;
    }

    // Static libraries drop unreferenced objects, so the generated resource
    // initializer must be referenced explicitly
    static const bool resourcesReady = [] {
        initStyleResources();
        return true;
    }();
    Q_UNUSED(resourcesReady)

    const StyleMode resolved = resolveMode(mode);
    const Tokens& tokens = resolved == StyleMode::Dark ? darkTokens() : lightTokens();

    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QApplication::setPalette(makePalette(tokens));

    if (QFile file(QStringLiteral(":/cad_common/styles/style.qss"));
        file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app->setStyleSheet(substituteTokens(QString::fromUtf8(file.readAll()), tokens));
    }
}

} // namespace cad_common::style

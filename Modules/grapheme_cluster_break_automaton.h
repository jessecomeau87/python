typedef enum {
    CR,
    LF,
    Control,
    Extend,
    ZWJ,
    Regional_Indicator,
    Prepend,
    SpacingMark,
    L,
    V,
    T,
    LV,
    LVT,
    E_Base,
    E_Modifier,
    Glue_After_Zwj,
    E_Base_GAZ,
    Any,
    eot
} GraphemeClusterBreakType;

typedef enum {
    STATE_BREAK,
    STATE_sot,
    STATE_eot,
    STATE_CR,
    STATE_LF,
    STATE_Control,
    STATE_L,
    STATE_V_or_LV,
    STATE_T_or_LVT,
    STATE_Prepend,
    STATE_ZWJ,
    STATE_Emoji,
    STATE_RI_1,
    STATE_RI_2,
    STATE_Any,
} GCBState;

static const GCBState GRAPH_CLUSTER_AUTOMATON[15][19] = {
        [STATE_BREAK] = {STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK,
                         STATE_BREAK},
        [STATE_sot] = {[CR] = STATE_CR,
                       [LF] = STATE_LF,
                       [Control] = STATE_Control,
                       [Extend] = STATE_Any,
                       [ZWJ] = STATE_ZWJ,
                       [Regional_Indicator] = STATE_RI_1,
                       [Prepend] = STATE_Prepend,
                       [SpacingMark] = STATE_Any,
                       [L] = STATE_L,
                       [V] = STATE_V_or_LV,
                       [T] = STATE_T_or_LVT,
                       [LV] = STATE_V_or_LV,
                       [LVT] = STATE_T_or_LVT,
                       [E_Base] = STATE_Emoji,
                       [E_Modifier] = STATE_Any,
                       [Glue_After_Zwj] = STATE_Any,
                       [E_Base_GAZ] = STATE_Emoji,
                       [Any] = STATE_Any,
                       [eot] = STATE_eot},
        [STATE_eot] = {[CR] = STATE_BREAK,
                       [LF] = STATE_BREAK,
                       [Control] = STATE_BREAK,
                       [Extend] = STATE_BREAK,
                       [ZWJ] = STATE_BREAK,
                       [Regional_Indicator] = STATE_BREAK,
                       [Prepend] = STATE_BREAK,
                       [SpacingMark] = STATE_BREAK,
                       [L] = STATE_BREAK,
                       [V] = STATE_BREAK,
                       [T] = STATE_BREAK,
                       [LV] = STATE_BREAK,
                       [LVT] = STATE_BREAK,
                       [E_Base] = STATE_BREAK,
                       [E_Modifier] = STATE_BREAK,
                       [Glue_After_Zwj] = STATE_BREAK,
                       [E_Base_GAZ] = STATE_BREAK,
                       [Any] = STATE_BREAK,
                       [eot] = STATE_BREAK},
        [STATE_CR] = {[CR] = STATE_BREAK,
                      [LF] = STATE_LF,
                      [Control] = STATE_BREAK,
                      [Extend] = STATE_BREAK,
                      [ZWJ] = STATE_BREAK,
                      [Regional_Indicator] = STATE_BREAK,
                      [Prepend] = STATE_BREAK,
                      [SpacingMark] = STATE_BREAK,
                      [L] = STATE_BREAK,
                      [V] = STATE_BREAK,
                      [T] = STATE_BREAK,
                      [LV] = STATE_BREAK,
                      [LVT] = STATE_BREAK,
                      [E_Base] = STATE_BREAK,
                      [E_Modifier] = STATE_BREAK,
                      [Glue_After_Zwj] = STATE_BREAK,
                      [E_Base_GAZ] = STATE_BREAK,
                      [Any] = STATE_BREAK,
                      [eot] = STATE_BREAK},
        [STATE_LF] = {[CR] = STATE_BREAK,
                      [LF] = STATE_BREAK,
                      [Control] = STATE_BREAK,
                      [Extend] = STATE_BREAK,
                      [ZWJ] = STATE_BREAK,
                      [Regional_Indicator] = STATE_BREAK,
                      [Prepend] = STATE_BREAK,
                      [SpacingMark] = STATE_BREAK,
                      [L] = STATE_BREAK,
                      [V] = STATE_BREAK,
                      [T] = STATE_BREAK,
                      [LV] = STATE_BREAK,
                      [LVT] = STATE_BREAK,
                      [E_Base] = STATE_BREAK,
                      [E_Modifier] = STATE_BREAK,
                      [Glue_After_Zwj] = STATE_BREAK,
                      [E_Base_GAZ] = STATE_BREAK,
                      [Any] = STATE_BREAK,
                      [eot] = STATE_BREAK},
        [STATE_Control] = {[CR] = STATE_BREAK,
                           [LF] = STATE_BREAK,
                           [Control] = STATE_BREAK,
                           [Extend] = STATE_BREAK,
                           [ZWJ] = STATE_BREAK,
                           [Regional_Indicator] = STATE_BREAK,
                           [Prepend] = STATE_BREAK,
                           [SpacingMark] = STATE_BREAK,
                           [L] = STATE_BREAK,
                           [V] = STATE_BREAK,
                           [T] = STATE_BREAK,
                           [LV] = STATE_BREAK,
                           [LVT] = STATE_BREAK,
                           [E_Base] = STATE_BREAK,
                           [E_Modifier] = STATE_BREAK,
                           [Glue_After_Zwj] = STATE_BREAK,
                           [E_Base_GAZ] = STATE_BREAK,
                           [Any] = STATE_BREAK,
                           [eot] = STATE_BREAK},
        [STATE_L] = {[CR] = STATE_BREAK,
                     [LF] = STATE_BREAK,
                     [Control] = STATE_BREAK,
                     [Extend] = STATE_Any,
                     [ZWJ] = STATE_ZWJ,
                     [Regional_Indicator] = STATE_BREAK,
                     [Prepend] = STATE_BREAK,
                     [SpacingMark] = STATE_Any,
                     [L] = STATE_L,
                     [V] = STATE_V_or_LV,
                     [T] = STATE_BREAK,
                     [LV] = STATE_V_or_LV,
                     [LVT] = STATE_T_or_LVT,
                     [E_Base] = STATE_BREAK,
                     [E_Modifier] = STATE_BREAK,
                     [Glue_After_Zwj] = STATE_BREAK,
                     [E_Base_GAZ] = STATE_BREAK,
                     [Any] = STATE_BREAK,
                     [eot] = STATE_BREAK},
        [STATE_V_or_LV] = {[CR] = STATE_BREAK,
                           [LF] = STATE_BREAK,
                           [Control] = STATE_BREAK,
                           [Extend] = STATE_Any,
                           [ZWJ] = STATE_ZWJ,
                           [Regional_Indicator] = STATE_BREAK,
                           [Prepend] = STATE_BREAK,
                           [SpacingMark] = STATE_Any,
                           [L] = STATE_BREAK,
                           [V] = STATE_V_or_LV,
                           [T] = STATE_T_or_LVT,
                           [LV] = STATE_BREAK,
                           [LVT] = STATE_BREAK,
                           [E_Base] = STATE_BREAK,
                           [E_Modifier] = STATE_BREAK,
                           [Glue_After_Zwj] = STATE_BREAK,
                           [E_Base_GAZ] = STATE_BREAK,
                           [Any] = STATE_BREAK,
                           [eot] = STATE_BREAK},
        [STATE_T_or_LVT] = {[CR] = STATE_BREAK,
                            [LF] = STATE_BREAK,
                            [Control] = STATE_BREAK,
                            [Extend] = STATE_Any,
                            [ZWJ] = STATE_ZWJ,
                            [Regional_Indicator] = STATE_BREAK,
                            [Prepend] = STATE_BREAK,
                            [SpacingMark] = STATE_Any,
                            [L] = STATE_BREAK,
                            [V] = STATE_BREAK,
                            [T] = STATE_T_or_LVT,
                            [LV] = STATE_BREAK,
                            [LVT] = STATE_BREAK,
                            [E_Base] = STATE_BREAK,
                            [E_Modifier] = STATE_BREAK,
                            [Glue_After_Zwj] = STATE_BREAK,
                            [E_Base_GAZ] = STATE_BREAK,
                            [Any] = STATE_BREAK,
                            [eot] = STATE_BREAK},
        [STATE_Prepend] = {[CR] = STATE_BREAK,
                           [LF] = STATE_BREAK,
                           [Control] = STATE_BREAK,
                           [Extend] = STATE_Any,
                           [ZWJ] = STATE_ZWJ,
                           [Regional_Indicator] = STATE_RI_1,
                           [Prepend] = STATE_Prepend,
                           [SpacingMark] = STATE_Any,
                           [L] = STATE_L,
                           [V] = STATE_V_or_LV,
                           [T] = STATE_T_or_LVT,
                           [LV] = STATE_V_or_LV,
                           [LVT] = STATE_T_or_LVT,
                           [E_Base] = STATE_Emoji,
                           [E_Modifier] = STATE_Any,
                           [Glue_After_Zwj] = STATE_Any,
                           [E_Base_GAZ] = STATE_Emoji,
                           [Any] = STATE_Any,
                           [eot] = STATE_BREAK},
        [STATE_ZWJ] = {[CR] = STATE_BREAK,
                       [LF] = STATE_BREAK,
                       [Control] = STATE_BREAK,
                       [Extend] = STATE_Any,
                       [ZWJ] = STATE_ZWJ,
                       [Regional_Indicator] = STATE_BREAK,
                       [Prepend] = STATE_BREAK,
                       [SpacingMark] = STATE_Any,
                       [L] = STATE_BREAK,
                       [V] = STATE_BREAK,
                       [T] = STATE_BREAK,
                       [LV] = STATE_BREAK,
                       [LVT] = STATE_BREAK,
                       [E_Base] = STATE_BREAK,
                       [E_Modifier] = STATE_BREAK,
                       [Glue_After_Zwj] = STATE_Any,
                       [E_Base_GAZ] = STATE_Emoji,
                       [Any] = STATE_BREAK,
                       [eot] = STATE_BREAK},
        [STATE_Emoji] = {[CR] = STATE_BREAK,
                         [LF] = STATE_BREAK,
                         [Control] = STATE_BREAK,
                         [Extend] = STATE_Emoji,
                         [ZWJ] = STATE_ZWJ,
                         [Regional_Indicator] = STATE_BREAK,
                         [Prepend] = STATE_BREAK,
                         [SpacingMark] = STATE_Any,
                         [L] = STATE_BREAK,
                         [V] = STATE_BREAK,
                         [T] = STATE_BREAK,
                         [LV] = STATE_BREAK,
                         [LVT] = STATE_BREAK,
                         [E_Base] = STATE_BREAK,
                         [E_Modifier] = STATE_Any,
                         [Glue_After_Zwj] = STATE_BREAK,
                         [E_Base_GAZ] = STATE_BREAK,
                         [Any] = STATE_BREAK,
                         [eot] = STATE_BREAK},
        [STATE_RI_1] = {[CR] = STATE_BREAK,
                        [LF] = STATE_BREAK,
                        [Control] = STATE_BREAK,
                        [Extend] = STATE_Any,
                        [ZWJ] = STATE_ZWJ,
                        [Regional_Indicator] = STATE_RI_2,
                        [Prepend] = STATE_BREAK,
                        [SpacingMark] = STATE_Any,
                        [L] = STATE_BREAK,
                        [V] = STATE_BREAK,
                        [T] = STATE_BREAK,
                        [LV] = STATE_BREAK,
                        [LVT] = STATE_BREAK,
                        [E_Base] = STATE_BREAK,
                        [E_Modifier] = STATE_BREAK,
                        [Glue_After_Zwj] = STATE_BREAK,
                        [E_Base_GAZ] = STATE_BREAK,
                        [Any] = STATE_BREAK,
                        [eot] = STATE_BREAK},
        [STATE_RI_2] = {[CR] = STATE_BREAK,
                        [LF] = STATE_BREAK,
                        [Control] = STATE_BREAK,
                        [Extend] = STATE_Any,
                        [ZWJ] = STATE_ZWJ,
                        [Regional_Indicator] = STATE_BREAK,
                        [Prepend] = STATE_BREAK,
                        [SpacingMark] = STATE_Any,
                        [L] = STATE_BREAK,
                        [V] = STATE_BREAK,
                        [T] = STATE_BREAK,
                        [LV] = STATE_BREAK,
                        [LVT] = STATE_BREAK,
                        [E_Base] = STATE_BREAK,
                        [E_Modifier] = STATE_BREAK,
                        [Glue_After_Zwj] = STATE_BREAK,
                        [E_Base_GAZ] = STATE_BREAK,
                        [Any] = STATE_BREAK,
                        [eot] = STATE_BREAK},
        [STATE_Any] = {[CR] = STATE_BREAK,
                       [LF] = STATE_BREAK,
                       [Control] = STATE_BREAK,
                       [Extend] = STATE_Any,
                       [ZWJ] = STATE_ZWJ,
                       [Regional_Indicator] = STATE_BREAK,
                       [Prepend] = STATE_BREAK,
                       [SpacingMark] = STATE_Any,
                       [L] = STATE_BREAK,
                       [V] = STATE_BREAK,
                       [T] = STATE_BREAK,
                       [LV] = STATE_BREAK,
                       [LVT] = STATE_BREAK,
                       [E_Base] = STATE_BREAK,
                       [E_Modifier] = STATE_BREAK,
                       [Glue_After_Zwj] = STATE_BREAK,
                       [E_Base_GAZ] = STATE_BREAK,
                       [Any] = STATE_BREAK,
                       [eot] = STATE_BREAK},
};

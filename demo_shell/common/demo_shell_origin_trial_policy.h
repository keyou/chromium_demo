#ifndef CONTENT_DEMO_SHELL_COMMON_DEMO_SHELL_ORIGIN_TRIAL_POLICY_H_
#define CONTENT_DEMO_SHELL_COMMON_DEMO_SHELL_ORIGIN_TRIAL_POLICY_H_

#include "third_party/blink/public/common/origin_trials/origin_trial_policy.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace content{
class DemoShellOriginTrialPolicy: public blink::OriginTrialPolicy{
public:
    DemoShellOriginTrialPolicy();
    ~DemoShellOriginTrialPolicy() override;
    // blink::OriginTrialPolicy interface
    bool IsOriginTrialsSupported() const override;
    base::StringPiece GetPublicKey() const override;
    bool IsOriginSecure(const GURL& url) const override;

 private:
    base::StringPiece public_key_;

    DISALLOW_COPY_AND_ASSIGN(DemoShellOriginTrialPolicy);
};
}

#endif //CONTENT_DEMO_SHELL_COMMON_DEMO_SHELL_ORIGIN_TRIAL_POLICY_H_
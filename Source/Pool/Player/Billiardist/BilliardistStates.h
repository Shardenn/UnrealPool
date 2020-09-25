#pragma once

UENUM(BlueprintType)
enum class FBilliardistState : uint8
{
    WALKING             UMETA(DisplayName = "Walking"),     // just walking around the table, examining
    PICKING             UMETA(DisplayName = "Picking"),     // if we are playing RU billiard, we can pick any ball for the shot.
                                                        // if 8ball, we can either pick cue ball and start aiming,
                                                        // or pick any other ball to do a called shot
    AIMING              UMETA(DisplayName = "Aiming"),      // when a ball is picked, we aim for the shot, holding the cue near the ball
    OBSERVING           UMETA(DisplayName = "Observing"),  // observing the balls after a shot
    EXAMINING           UMETA(DisplayName = "Examinging"),  // watching from the top of the table
    FORCED_TO_NAME_SHOT UMETA(DisplayName = "Forced to name shot") // on last ball pocketing a player must name the pocket and the ball - name shot
};
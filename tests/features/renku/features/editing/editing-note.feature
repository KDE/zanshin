Feature: Editing notes
  As an organized person
  I can edit a previously created note
  In order to improve it, logging more ideas

  Scenario: Editing a note text
    Given I display the "Inbox" page
    And there is an item named "A note about nothing interesting" in the central list
    When I open the item in the editor
    And I change the editor text to "This is a boring note"
    And I open the item in the editor again
    Then the editor shows "This is a boring note" as text

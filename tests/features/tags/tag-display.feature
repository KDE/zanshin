Feature: Tag content
  As someone collecting tasks and notes
  I can display a tag
  In order to see the tasks and notes associated to it

@wip
  Scenario: Tag tasks and notes appear in the corresponding page
    Given I display the "Tags / Philosophy" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                                                   |
       |"Capital in the Twenty-First Century" by Thomas Piketty    |
